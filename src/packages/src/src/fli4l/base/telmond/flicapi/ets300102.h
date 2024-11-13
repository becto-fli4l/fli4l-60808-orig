/*****************************************************************************
 *  @file flicapi/ets300102.h
 *  Strcutures describing parts of the ETS 300 102-1 standard which serves as
 *  the base of the ISDN/CAPI layer.
 *
 *  Copyright (c) 2012-2016 The fli4l team
 *
 *  This file is part of fli4l.
 *
 *  fli4l is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  fli4l is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with fli4l.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Last Update: $Id$
 *****************************************************************************
 */

#ifndef FLICAPI_ETS300102_H_
#define FLICAPI_ETS300102_H_

/**
 * Represents the structure "Called Party Number", ETS 300 102-1, §4.5.8.
 * It is referenced by CAPI standard, Part I, pp. 80-81.
 */
struct ETS300102_CalledPartyNumber {
    /**
     * The numbering plan used.
     */
    unsigned char numberingPlan : 4;
    /**
     * The type of number.
     */
    unsigned char numberType : 3;
    /**
     * The extension bit of octet 3.
     */
    unsigned char ext1 : 1;
    /**
     * The digits of the number.
     */
    char digits[1];
};

/**
 * Represents the structure "Calling Party Number", ETS 300 102-1, §4.5.10.
 * It is referenced by CAPI standard, Part I, pp. 81-82.
 */
struct ETS300102_CallingPartyNumber {
    /**
     * The numbering plan used.
     */
    unsigned char numberingPlan : 4;
    /**
     * The type of number.
     */
    unsigned char numberType : 3;
    /**
     * The extension bit of octet 3.
     */
    unsigned char ext1 : 1;
    /**
     * The screening indicator (not used for ISDN).
     */
    unsigned char screening : 2;
    /**
     * Reserved.
     */
    unsigned char spare1 : 3;
    /**
     * The presentation indicator.
     */
    unsigned char presentation : 2;
    /**
     * The extension bit of octet 3a.
     */
    unsigned char ext2 : 1;
    /**
     * The digits of the number.
     */
    char digits[1];
};

/**
 * @name Number types
 * @{
 */
#define ETS300102_NUMBER_TYPE_UNKNOWN 0
#define ETS300102_NUMBER_TYPE_INTERNATIONAL 1
#define ETS300102_NUMBER_TYPE_NATIONAL 2
#define ETS300102_NUMBER_TYPE_NETWORK_SPECIFIC 3
#define ETS300102_NUMBER_TYPE_SUBSCRIBER 4
#define ETS300102_NUMBER_TYPE_ABBREVIATED 6
#define ETS300102_NUMBER_TYPE_EXTENSION 7
/**
 * @}
 */

/**
 * @name Numbering plans
 * @{
 */
#define ETS300102_NUMBERING_PLAN_UNKNOWN 0
#define ETS300102_NUMBERING_PLAN_ISDN 1
#define ETS300102_NUMBERING_PLAN_DATA 3
#define ETS300102_NUMBERING_PLAN_TELEX 4
#define ETS300102_NUMBERING_PLAN_NATIONAL 8
#define ETS300102_NUMBERING_PLAN_PRIVATE 9
#define ETS300102_NUMBERING_PLAN_EXTENSION 15
/**
 * @}
 */

/**
 * @name Presentation indicators
 * @{
 */
#define ETS300102_PRESENTATION_ALLOWED 0
#define ETS300102_PRESENTATION_RESTRICTED 1
#define ETS300102_PRESENTATION_NUMBER_NOT_AVAILABLE 2
/**
 * @}
 */

#endif
