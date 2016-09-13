//==============================================================================
//
// Title:		EXTDEV_DRIVER_DDS.h
// Purpose:		A short description of the interface.
//
// Created on:	23.09.2014 at 16:25:09 by EDV.
// Copyright:	. All Rights Reserved.
//
//==============================================================================

#ifndef __EXTDEV_DRIVER_DDS_H__
#define __EXTDEV_DRIVER_DDS_H__

#ifdef __cplusplus
    extern "C" {
#endif


int EXTDEV_DDS_THREAD_transmitFrequency(t_sequence *seq, t_gpibCommand *g,t_ext_device *extdev);


#endif  /* ndef __EXTDEV_DRIVER_DDS_H__ */
