//**************************************************************************\
//* This file is property of and copyright by the ALICE Project            *\
//* ALICE Experiment at CERN, All rights reserved.                         *\
//*                                                                        *\
//* Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *\
//*                  for The ALICE HLT Project.                            *\
//*                                                                        *\
//* Permission to use, copy, modify and distribute this software and its   *\
//* documentation strictly for non-commercial purposes is hereby granted   *\
//* without fee, provided that the above copyright notice appears in all   *\
//* copies and that both the copyright notice and this permission notice   *\
//* appear in the supporting documentation. The authors make no claims     *\
//* about the suitability of this software for any purpose. It is          *\
//* provided "as is" without express or implied warranty.                  *\
//**************************************************************************

/// \file GPUTRDTrack.cxx
/// \author Ole Schmidt, Sergey Gorbunov

#include "GPUTRDTrack.h"
#if !defined(GPU_TRD_TRACK_O2)
#include "GPUTRDInterfaces.h"
#endif

using namespace GPUCA_NAMESPACE::gpu;

template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t()
{
  // default constructor
  initialize();
}

template <typename T>
GPUd() void GPUTRDTrack_t<T>::initialize()
{
  // set all members to their default values (needed since in-class initialization not possible with AliRoot)
  mChi2 = 0.f;
  mRefGlobalTrackId = 0;
  mCollisionId = -1;
  mFlags = 0;
  for (int i = 0; i < kNLayers; ++i) {
    mAttachedTracklets[i] = -1;
  }
}

#ifdef GPUCA_ALIROOT_LIB
#include "AliHLTExternalTrackParam.h"
#include "GPUTRDTrackData.h"

template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t(const AliHLTExternalTrackParam& t) : T(t)
{
  initialize();
}

template <typename T>
GPUd() void GPUTRDTrack_t<T>::ConvertTo(GPUTRDTrackDataRecord& t) const
{
  //------------------------------------------------------------------
  // convert to GPU structure
  //------------------------------------------------------------------
  t.mAlpha = T::getAlpha();
  t.fX = T::getX();
  t.fY = T::getY();
  t.fZ = T::getZ();
  t.fq1Pt = T::getQ2Pt();
  t.mSinPhi = T::getSnp();
  t.fTgl = T::getTgl();
  for (int i = 0; i < 15; i++) {
    t.fC[i] = T::getCov()[i];
  }
  t.fTPCTrackID = getRefGlobalTrackIdRaw();
  for (int i = 0; i < kNLayers; i++) {
    t.fAttachedTracklets[i] = getTrackletIndex(i);
  }
}

template <typename T>
GPUd() void GPUTRDTrack_t<T>::ConvertFrom(const GPUTRDTrackDataRecord& t)
{
  //------------------------------------------------------------------
  // convert from GPU structure
  //------------------------------------------------------------------
  T::set(t.fX, t.mAlpha, &(t.fY), t.fC);
  setRefGlobalTrackIdRaw(t.fTPCTrackID);
  mChi2 = 0.f;
  mFlags = 0;
  mCollisionId = -1;
  for (int iLayer = 0; iLayer < kNLayers; iLayer++) {
    mAttachedTracklets[iLayer] = t.fAttachedTracklets[iLayer];
  }
}

#endif

#if defined(GPUCA_HAVE_O2HEADERS)
#include "ReconstructionDataFormats/TrackTPCITS.h"
#include "DataFormatsTPC/TrackTPC.h"

template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t(const o2::dataformats::TrackTPCITS& t) : T(t)
{
  initialize();
}

template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t(const o2::tpc::TrackTPC& t) : T(t)
{
  initialize();
}

#endif

template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t(const GPUTRDTrack_t<T>& t)
  : T(t), mChi2(t.mChi2), mRefGlobalTrackId(t.mRefGlobalTrackId), mCollisionId(t.mCollisionId), mFlags(t.mFlags)
{
  // copy constructor
  for (int i = 0; i < kNLayers; ++i) {
    mAttachedTracklets[i] = t.mAttachedTracklets[i];
  }
}

template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t(const T& t) : T(t)
{
  // copy constructor from anything
  initialize();
}

template <typename T>
GPUd() GPUTRDTrack_t<T>& GPUTRDTrack_t<T>::operator=(const GPUTRDTrack_t<T>& t)
{
  // assignment operator
  if (&t == this) {
    return *this;
  }
  *(T*)this = t;
  mChi2 = t.mChi2;
  mRefGlobalTrackId = t.mRefGlobalTrackId;
  mCollisionId = t.mCollisionId;
  mFlags = t.mFlags;
  for (int i = 0; i < kNLayers; ++i) {
    mAttachedTracklets[i] = t.mAttachedTracklets[i];
  }
  return *this;
}

template <typename T>
GPUd() int GPUTRDTrack_t<T>::getNlayersFindable() const
{
  // returns number of layers in which the track is in active area of TRD
  int retVal = 0;
  for (int iLy = 0; iLy < kNLayers; iLy++) {
    if ((mFlags >> iLy) & 0x1) {
      ++retVal;
    }
  }
  return retVal;
}

template <typename T>
GPUd() int GPUTRDTrack_t<T>::getNtracklets() const
{
  // returns number of tracklets attached to this track
  int retVal = 0;
  for (int iLy = 0; iLy < kNLayers; ++iLy) {
    if (mAttachedTracklets[iLy] >= 0) {
      ++retVal;
    }
  }
  return retVal;
}

template <typename T>
GPUd() int GPUTRDTrack_t<T>::getNmissingConsecLayers(int iLayer) const
{
  // returns number of consecutive layers in which the track was
  // inside the deadzone up to (and including) the given layer
  int retVal = 0;
  while (!getIsFindable(iLayer)) {
    ++retVal;
    --iLayer;
    if (iLayer < 0) {
      break;
    }
  }
  return retVal;
}

#if !defined(GPUCA_GPUCODE) && !defined(GPU_TRD_TRACK_O2)
namespace GPUCA_NAMESPACE
{
namespace gpu
{
#ifdef GPUCA_ALIROOT_LIB // Instantiate AliRoot track version
template class GPUTRDTrack_t<trackInterface<AliExternalTrackParam>>;
#endif
#if defined(GPUCA_HAVE_O2HEADERS) && !defined(GPUCA_O2_LIB) // Instantiate O2 track version, for O2 this happens in GPUTRDTrackO2.cxx
template class GPUTRDTrack_t<trackInterface<o2::track::TrackParCov>>;
#endif
template class GPUTRDTrack_t<trackInterface<GPUTPCGMTrackParam>>; // Always instatiate GM track version
} // namespace gpu
} // namespace GPUCA_NAMESPACE
#endif
