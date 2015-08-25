/*
 * ShortStack for Raspberry Pi Example of applications with runtime
 * interface selection.
 *
 * See the rpi-ris.c file for more about this application example.
 * This file implements the dispatcher which enables one of several
 * interfaces.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */
#include "ShortStackDev.h"
#include "regularDev.h"
#include "ShortStackApi.h"
#include "ldv.h"

#define	INTERFACE_REGULAR	0
#define INTERFACE_DELUXE	1

/*
 * SECTION: Interface selection
 */

/*
 * Variable: interfaceId
 *
 * This example implementation uses a simple integer interface identifier.
 * Use LonSelectInterface to set the value of this identifier.
 */
static int interfaceId = INTERFACE_REGULAR;

/*
 * Function: SelectApplicationInterface
 *
 * Use this function to select the interface. A call to LonInit() or
 * LonReinit() must follow.
 */
void SelectApplicationInterface(int id)
{
	if (id == INTERFACE_REGULAR || id == INTERFACE_DELUXE) {
		interfaceId = id;
	}
}

/*
 * SECTION: IzoT Interface Interpreter Framework
 *
 * The following functions are called by the API and are normally implemented
 * by the IzoT Interface Interpreter. However, an application which uses
 * IzoT Interface Interpreter namespaces can intercept these calls, as done
 * here.
 */

/*
 * LonFrameworkInit
 *
 * The API calls this after opening the driver, but prior to all else.
 * This function decides which interface to use when the device starts.
 *
 */
void LonFrameworkInit(void)
{
	switch(interfaceId) {
	case INTERFACE_REGULAR:
		regularLonFrameworkInit();
		break;
	case INTERFACE_DELUXE:
		deluxeLonFrameworkInit();
		break;
	}
}

const LonByte* LonGetSiData(unsigned* pLength)
{
	const LonByte* result = NULL;

	switch(interfaceId) {
	case INTERFACE_REGULAR:
		result = regularLonGetSiData(pLength);
		break;
	case INTERFACE_DELUXE:
		result = deluxeLonGetSiData(pLength);
		break;
	}
	return result;
}

const LonByte* LonGetAppInitData(void)
{
	const LonByte* result = NULL;

	switch(interfaceId) {
	case INTERFACE_REGULAR:
		result = regularLonGetAppInitData();
		break;
	case INTERFACE_DELUXE:
		result = deluxeLonGetAppInitData();
		break;
	}
	return result;
}

void* LonGetNvTable(void)
{
	void* result = NULL;

	switch(interfaceId) {
	case INTERFACE_REGULAR:
		result = regularLonGetNvTable();
		break;
	case INTERFACE_DELUXE:
		result = deluxeLonGetNvTable();
		break;
	}
	return result;
}

unsigned LonGetNvCount(void)
{
	unsigned result = 0;
	switch(interfaceId) {
	case INTERFACE_REGULAR:
		result = regularLonGetNvCount();
		break;
	case INTERFACE_DELUXE:
		result = deluxeLonGetNvCount();
		break;
	}
	return result;
}

unsigned LonGetMtCount(void)
{
	unsigned result = 0;

	switch(interfaceId) {
	case INTERFACE_REGULAR:
		result = regularLonGetMtCount();
		break;
	case INTERFACE_DELUXE:
		result = deluxeLonGetMtCount();
		break;
	}
	return result;
}

LonUbits32 LonGetSignature(void)
{
	LonUbits32 result = 0;

	switch(interfaceId) {
	case INTERFACE_REGULAR:
		result = regularLonGetSignature();
		break;
	case INTERFACE_DELUXE:
		result = deluxeLonGetSignature();
		break;
	}
	return result;
}

/*
 * SECTION: Callbacks
 */
extern void regularLonResetOccurred(const LonResetNotification* const pResetNotification);
extern void deluxeLonResetOccurred(const LonResetNotification* const pResetNotification);

void LonResetOccurred(const LonResetNotification* const pResetNotification)
{
	switch(interfaceId) {
	case INTERFACE_REGULAR:
		regularLonResetOccurred(pResetNotification);
		break;
	case INTERFACE_DELUXE:
		deluxeLonResetOccurred(pResetNotification);
		break;
	}
}

extern void regularLonWink(void);
extern void deluxeLonWink(void);

void LonWink(void)
{
	switch(interfaceId) {
	case INTERFACE_REGULAR:
		regularLonWink();
		break;
	case INTERFACE_DELUXE:
		deluxeLonWink();
		break;
	}
}

extern void regularLonOffline(void);
extern void deluxeLonOffline(void);

void LonOffline(void)
{
	switch(interfaceId) {
	case INTERFACE_REGULAR:
		regularLonOffline();
		break;
	case INTERFACE_DELUXE:
		deluxeLonOffline();
		break;
	}
}

extern void regularLonOnline(void);
extern void deluxeLonOnline(void);

void LonOnline(void)
{
	switch(interfaceId) {
	case INTERFACE_REGULAR:
		regularLonOnline();
		break;
	case INTERFACE_DELUXE:
		deluxeLonOnline();
		break;
	}
}

extern void regularLonServicePinPressed(void);
extern void deluxeLonServicePinPressed(void);

void LonServicePinPressed(void)
{
	switch(interfaceId) {
	case INTERFACE_REGULAR:
		regularLonServicePinPressed();
		break;
	case INTERFACE_DELUXE:
		deluxeLonServicePinPressed();
		break;
	}
}

extern void regularLonServicePinHeld(void);
extern void deluxeLonServicePinHeld(void);

void LonServicePinHeld(void)
{
	switch(interfaceId) {
	case INTERFACE_REGULAR:
		regularLonServicePinHeld();
		break;
	case INTERFACE_DELUXE:
		deluxeLonServicePinHeld();
		break;
	}
}

extern void regularLonNvUpdateOccurred(
    const unsigned index,
    const LonReceiveAddress* const pSourceAddress
);
extern void deluxeLonNvUpdateOccurred(
    const unsigned index,
    const LonReceiveAddress* const pSourceAddress
);

void LonNvUpdateOccurred(
    const unsigned index,
    const LonReceiveAddress* const pSourceAddress
)
{
	switch(interfaceId) {
	case INTERFACE_REGULAR:
		regularLonNvUpdateOccurred(index, pSourceAddress);
		break;
	case INTERFACE_DELUXE:
		deluxeLonNvUpdateOccurred(index, pSourceAddress);
		break;
	}
}

extern void regularLonNvUpdateCompleted(const unsigned index, const LonBool success);
extern void deluxeLonNvUpdateCompleted(const unsigned index, const LonBool success);

void LonNvUpdateCompleted(const unsigned index, const LonBool success)
{
	switch(interfaceId) {
	case INTERFACE_REGULAR:
		regularLonNvUpdateCompleted(index, success);
		break;
	case INTERFACE_DELUXE:
		deluxeLonNvUpdateCompleted(index, success);
		break;
	}
}

extern const unsigned regularLonGetCurrentNvSize(const unsigned nvIndex);
extern const unsigned deluxeLonGetCurrentNvSize(const unsigned nvIndex);

const unsigned LonGetCurrentNvSize(const unsigned nvIndex)
{
    unsigned result = 0;
	switch(interfaceId) {
	case INTERFACE_REGULAR:
		result = regularLonGetCurrentNvSize(nvIndex);
		break;
	case INTERFACE_DELUXE:
		result = deluxeLonGetCurrentNvSize(nvIndex);
		break;
	}
    return result;
}

extern void* regularLonGetFile(int fileIndex, unsigned* pSize);
extern void* deluxeLonGetFile(int fileIndex, unsigned* pSize);

void* LonGetFile(int fileIndex, unsigned* pSize) {
	void* result = NULL;

	switch(interfaceId) {
	case INTERFACE_REGULAR:
		result = regularLonGetFile(fileIndex, pSize);
		break;
	case INTERFACE_DELUXE:
		result = deluxeLonGetFile(fileIndex, pSize);
		break;
	}
	return result;
}

