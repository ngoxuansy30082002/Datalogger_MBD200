/*********************************************************************
 *
 *       Emulating Data EEPROM for PIC32 microcontrollers
 *
 *********************************************************************
 * FileName:        dee_emulation_pic32.c
 * Dependencies:
 * Processor:       PIC32
 *
 * Complier:        MPLAB C32
 *                  MPLAB IDE
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the ?Company?) for its PIC32 Microcontroller is intended
 * and supplied to you, the Company?s customer, for use solely and
 * exclusively on Microchip PIC32 Microcontroller products.
 * The software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN ?AS IS? CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *$Id: $
 *
 **********************************************************************/
#include "dee_emulation.h"
//
//For the DEE emulation operation 3 Pages should be allocated in the program memory.
const uint32_t eedata_addr1[NUM_PAGE1][NUMBER_OF_INSTRUCTIONS_IN_PAGE] __attribute__((section("flash_data1"), address(EEDATA_ADDRESS1), space(prog)));
const uint32_t eedata_addr2[NUM_PAGE2][NUMBER_OF_INSTRUCTIONS_IN_PAGE] __attribute__((section("flash_data2"), address(EEDATA_ADDRESS2), space(prog)));

#define EEDATA_PAGE_ADDR(page) \
    ((page) < NUM_PAGE1 ? \
        (uint32_t)&eedata_addr1[(page)][0] : \
        (uint32_t)&eedata_addr2[(page) - NUM_PAGE1][0])


static int AddrIndex = 0;
static uint32_t ActivePage = 0;
static uint32_t CurrentPage = 0;
static uint32_t LowerAddress = 0; // to identify the read/write pointer address location
DATA_EE_FLAGS dataEEFlags; //Flags for the error/warning condition. 

/****************************************************************************
 * Function:        GetPageStatus
 *
 * PreCondition:    None
 *
 * Input:           page : Page number
 *                  field : Status field
 *
 * Output:          Right justified bit value representing selected Status
 *                  Field value
 *
 * Side Effects:    None
 *
 * Overview:        This routine returns the page status for the selected page, for the
 *                  selected field. The return value is right shifted into LSB position.
 *
 * Note:            This is a private function.
 *****************************************************************************/
char inline GetPageStatus(unsigned char page, unsigned char field) {
    uint32_t metaData;
    NVM_Read(&metaData, sizeof (metaData), EEDATA_PAGE_ADDR(page - 1));
    return ((metaData >> field) & 0x01);
}

/****************************************************************************
 * Function:        ErasePage
 *
 * PreCondition:    None
 *
 * Input:           page : Page number
 *
 * Output:          None
 *
 * Side Effects:    Generates CPU stall during program/erase operations
 *                  
 * Overview:        This routine erases the selected page and update the status 
 *                  bits by incrementing the erase count.
 *
 * Note:            This is private function.
 *****************************************************************************/
PAGE_STATUS ErasePage(unsigned char page) {
    uint32_t currentStatus;
    uint32_t metaData;

    NVM_Read(&metaData, sizeof (metaData), EEDATA_PAGE_ADDR(page - 1));
    currentStatus = (0x0000FFFF & metaData) + 1; //erase status & increment the erase count.

#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
    SYS_CONSOLE_PRINT("currentStatus %d\r\n", currentStatus);
#endif

    if (currentStatus > ERASE_WRITE_CYCLE_MAX) { // Mark page as expired
        NVM_WordWrite(0xFFEFFFFF, EEDATA_PAGE_ADDR(page - 1)); //page expired
        while (NVM_IsBusy());
        SetPageExpiredPage(1);
        return PAGE_STATUS_EXPIRED;
    } else {
        //        unsigned char *address = (char *) eedata_addr[page - 1];
        //        uint32_t i = 0;
        //        while (i < NUMBER_OF_INSTRUCTIONS_IN_PAGE) {
        //            NVM_PageErase(address);
        //            while (NVM_IsBusy());
        //
        //            address += PIC32MZ_PAGE_SIZE;
        //            i += (PIC32MZ_PAGE_SIZE / 4);
        //        }
        NVM_PageErase(EEDATA_PAGE_ADDR(page - 1));
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("EEDATA_PAGE_ADDR %x\r\n", EEDATA_PAGE_ADDR(page - 1));
#endif

        while (NVM_IsBusy());
        currentStatus = currentStatus | 0xFFFF0000;
        NVM_WordWrite(currentStatus, EEDATA_PAGE_ADDR(page - 1)); //update the new status bits (not update erase count)
        while (NVM_IsBusy());

        //        uint32_t word;
        //        for (int i = 1; i < NUMBER_OF_INSTRUCTIONS_IN_PAGE; i++) {
        //            NVM_Read(&word, sizeof (word), EEDATA_PAGE_ADDR(page - 1) + (i * 4));
        //            if (word != 0xFFFFFFFF) {
        //                SYS_CONSOLE_MESSAGE("111 ");
        //            }
        //        }
    }

    //return 0;
    //
    //if ((currentStatus & 0xFFFF) == ERASE_WRITE_CYCLE_MAX) {
    //    NVM_WordWrite(currentStatus & 0xFFEFFFFF, EEDATA_PAGE_ADDR(page - 1)); //page expired
    //    while (NVM_IsBusy());
    //} else {
    //    NVM_PageErase(EEDATA_PAGE_ADDR(page - 1));
    //    while (NVM_IsBusy());
    //    NVM_WordWrite(currentStatus, EEDATA_PAGE_ADDR(page - 1)); //update the status bits
    //    while (NVM_IsBusy());
    //}

    return PAGE_STATUS_VALID;
}

/****************************************************************************
 * Function:        PrevPage
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          return the previous page
 *
 * Side Effects:    None
 *
 * Overview:        This routine gives you the page before the current page
 *
 * Note:            This is a private function.
 *****************************************************************************/

uint32_t PrevPage(uint32_t currentPage) {
    uint32_t prevPage;

    prevPage = currentPage - 1;
    if (currentPage == 1)
        prevPage = NUM_DATA_EE_PAGES;
    return prevPage;
}

/****************************************************************************
 * Function:        GetNextAvailCount
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Page offset to next available location
 *
 * Side Effects:    None
 *
 * Overview:        This routine finds the current page and performs a backward search to find
 *                  the first available location. The available location is determined by
 *                  reading a 0xFFFF in the address region. The returned value can be added
 *                  to the first address in page to compute the available address. A return
 *                  value of 0xFFFF means the entire page was filled which is an error condition.
 *                  This routine can be called by the user to determine how full the current
 *                  page is prior to a pack.
 *
 * Note:            This is a public function.
 *****************************************************************************/
uint32_t GetNextAvailCount(void) {
    int i = 0;
    uint32_t addrEEval;
    uint32_t nextAvailCount = 0;

    if (CurrentPage == 0) {
        SetPageCorruptStatus(1);
    } else // A Current page is located.
    {
        i = AddrIndex >> 2;
        nextAvailCount = AddrIndex;
        do {
            NVM_Read(&addrEEval, sizeof (addrEEval), EEDATA_PAGE_ADDR(CurrentPage - 1) + 16 + (i * 4));

            if ((addrEEval >> 16) == 0xFFFF) {
                LowerAddress = 0;
                break;
            } else if ((addrEEval & 0xFFFF) == 0xFFFF) {
                LowerAddress = 1;
                break;
            }
            i++;
            nextAvailCount += 4;
        } while (nextAvailCount < DATA_OFFSET);
    }

    if ((nextAvailCount == DATA_OFFSET) && (ActivePage > 1)) {
        nextAvailCount = 0xFFFF; // Error - No available locations
    }

    return (nextAvailCount);
}

/****************************************************************************
 * Function:        EmulationCheckSum
 *
 * PreCondition:    None
 *
 * Input:           data
 *
 * Output:          return the checksum
 *
 * Side Effects:    None
 *
 * Overview:        This routine gives you the checksum for the data. Checksum will be
 *                  stored in upper 6 bits of 16 bit address value. This is mainly used
 *                  for the data integrity purpose. 
 *
 * Note:            This is a private function.
 *****************************************************************************/

uint32_t EmulationCheckSum(uint32_t data) {
    uint32_t sum;

    sum = 4 + (0x3 & data); // Start with seed 4, AB
    // sum = 0x3 & data;        // Original.

    while ((data = data >> 2))
        sum = sum + (0x3 & data);
    return sum;
}

/*****************************************************************************
 *	Function:	DataEEVerifyPage
 *
 *	Overview:	Verification of the whole EEdata page.
 * 				There are several possible outcomes:
 *
 *	Precondition:	None.
 *
 * 	Input:          page : Page number in eedata_addr.
 *
 *	Output:
 *			11.	The memory contain all 0xFFFFFFFF
 *	    		    as when erased during  programming.
 *					The page may be initialized
 *					for DataEE operation without erase.
 *			12.	The memory contain all 0x00000000
 *	        		as when initialized as an array by the compiler.
 *	        		A NVM erase will be needed.
 *			 3.	The memory contain random data not beeing a DataEE page.
 *			 2.	The memory contain a DataEE page that is expired.
 *			 6.	The memory contain a DataEE page with corrupted contents.
 *			 9.	One or more entries has failed the Checksum test,
 *          		number of failed words in the high end of dataEEFlags
 *		    		page is not expired and has a valid Erase counter.
 *			 0.	Page contain a valid DataEE datastructure.
 *			13. Page contain DataEE data, but is marked as Not Active. A Page Erase may be used.
 *	Arne Bergseth	17. November 2013
 ***************************************************************************/
PAGE_STATUS DataEEVerifyPage(unsigned char Page) {
    uint32_t addLoc;
    uint32_t dataLoc;
    int addrIndex, ErrCnt = 0;
    int i, p = Page - 1;

    uint32_t metaData;
    NVM_Read(&metaData, sizeof (metaData), EEDATA_PAGE_ADDR(p));

#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
    SYS_CONSOLE_PRINT("Page %d , metadata: %x\r\n", Page, metaData);
#endif

    switch (metaData) {
        case 0x00000000:
        {
            // compiler init page, need erase by nvm
            uint32_t word;
            for (i = 1; i < NUMBER_OF_INSTRUCTIONS_IN_PAGE; i++) {
                NVM_Read(&word, sizeof (word), EEDATA_PAGE_ADDR(p) + (i * 4));
                if (word != 0x00000000) {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: 111\r\n", __func__);
#endif

                    SetPageCorruptStatus(1); // Page contain garbage data, page is corrupt
                    return PAGE_STATUS_CORRUPT;
                }
            }
            return PAGE_STATUS_INITIALIZED;
            break;
        }
        case 0xFFFFFFFF:
        {
            // erased by nvm, ready for init 
            uint32_t word;
            for (i = 1; i < NUMBER_OF_INSTRUCTIONS_IN_PAGE; i++) {
                NVM_Read(&word, sizeof (word), EEDATA_PAGE_ADDR(p) + (i * 4));
                if (word != 0xFFFFFFFF) {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: 222\r\n", __func__);
#endif

                    SetPageCorruptStatus(1); // Page contain garbage data, page is corrupt
                    return PAGE_STATUS_CORRUPT;
                }
            }
            return PAGE_STATUS_ERASED;
            break;
        }
        default:
        {
            if ((metaData & 0xFFE10000) != 0xFFE10000) {
                // all flag status is detected
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("%s: 333\r\n", __func__);
#endif

                SetPageCorruptStatus(1);
                return PAGE_STATUS_CORRUPT;
            }
            if ((metaData & 0x0000FFFF) > ERASE_WRITE_CYCLE_MAX && (metaData & 0x0000FFFF) != 0xFFFF) {
                SetPageExpiredPage(1);
                return PAGE_STATUS_EXPIRED;
            }
            if ((metaData & 0x00100000) == 0) {
                // Check expired flag
                SetPageExpiredPage(1);
                return PAGE_STATUS_EXPIRED;
            }
            break;
        }
    }

    // Possible check for invalid combinations of Status bits.

    addLoc = (uint32_t) EEDATA_PAGE_ADDR(p) + 16;
    addrIndex = DATA_OFFSET - 4;
    do {
        // Scan the page (top down)
        uint32_t addrRead, dataRead;

        NVM_Read(&addrRead, sizeof (addrRead), (addrIndex + addLoc));
        dataLoc = (addrIndex * 2) + DATA_OFFSET + 4 + addLoc;
        if ((addrRead & 0xFFFF) == 0xFFFF) // address is Erased
        {
            NVM_Read(&dataRead, sizeof (dataRead), dataLoc);
            if (dataRead != 0xFFFFFFFF) // Check data word is erased.
            {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("%s: 444\r\n", __func__);
#endif

                SetPageCorruptStatus(1);
                ErrCnt++;
            }
        } else {
            //            addr = addrRead & 0x3FF; // Lower half word
            NVM_Read(&dataRead, sizeof (dataRead), dataLoc);
            if (((addrRead & 0xFC00) >> 0xA) == EmulationCheckSum(dataRead)) {
                if (GetPageStatus(Page, STATUS_ACTIVE) == PAGE_NOT_ACTIVE)
                    return PAGE_STATUS_INACTIVE;
            } else {
                SetCheckSumError(1);
                ErrCnt++;
            }
        }

        // Check the higher halfword of Address
        dataLoc = (addrIndex * 2) + DATA_OFFSET + addLoc;
        if ((addrRead & 0xFFFF0000) == 0xFFFF0000) // Erased
        {
            NVM_Read(&dataRead, sizeof (dataRead), dataLoc);
            if (dataRead != 0xFFFFFFFF) // Check Data word
            {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("%s: 555\r\n", __func__);
#endif

                SetPageCorruptStatus(1);
                ErrCnt++;
            }
        } else {
            //            addr = (addrRead >> 16) & 0x3FF;
            NVM_Read(&dataRead, sizeof (dataRead), dataLoc);
            if (((addrRead & 0xFC000000) >> 0x1A) == EmulationCheckSum(dataRead)) {
                if (GetPageStatus(Page, STATUS_ACTIVE) == PAGE_NOT_ACTIVE)
                    return PAGE_STATUS_INACTIVE;
            } else {
                SetCheckSumError(1);
                ErrCnt++;
            }
        }
        addrIndex -= 4;
    } while (addrIndex >= 0);
    if (ErrCnt) {
        dataEEFlags.errorCount += ErrCnt; // Accumulate
        if (GetPageCorruptStatus())
            return PAGE_STATUS_CORRUPT;
        if (GetCheckSumError())
            return PAGE_STATUS_CHECKSUM_ERROR;
    }
    return PAGE_STATUS_VALID;
}

/****************************************************************************
 * Function:        DataEEInit
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Check the dataEEFlags for the error status.
 *                  value 0 for success.
 *                  Value 2 for expired page.
 *                  Value 6 for page corrupt status.
 *                  Value 7 for write error.
 *                  Value 8 for Low voltage operation.
 *
 * Side Effects:    Data EE flags may be updated.
 *
 * Overview:        This routine finds an unexpired page to become an active page. It then
 *                  counts the number of active pages. If no active pages are found, the
 *                  first unexpired page is initialized for emulation. If one or two active pages
 *                  found, it assumes a reset occurred and the function does nothing. If
 *                  three active pages are found, it is assumes a reset occurred during a pack.
 *                  The page after current is erased and a pack is called. This
 *                  function must be called prior to any other operation.
 *
 * Note:            This is a public function.
 *****************************************************************************/
uint32_t DataEEInit(void) {
    uint32_t pageCount;
    uint32_t expiredPage = 0;
    uint32_t firstPage = 0;
    uint32_t CheckSumError = 0;
    PAGE_STATUS pageSts;

    dataEEFlags.val = 0;
    AddrIndex = 0;

    // Check all pages.
    for (pageCount = 1; pageCount <= NUM_DATA_EE_PAGES; pageCount++) {
        // Verification of the whole eedata page.
        pageSts = DataEEVerifyPage(pageCount);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("Page %d , status %d\r\n", pageCount, pageSts);
#endif

        switch (pageSts) {
            case PAGE_STATUS_VALID:
                break;

            case PAGE_STATUS_ERASED:
                // Page is erased.
                // Initialize page, Not Active, Erase count 0
                NVM_WordWrite(0xFFFF0000, EEDATA_PAGE_ADDR(pageCount - 1));
                while (NVM_IsBusy());
                break;
            case PAGE_STATUS_EXPIRED:
                //                expiredPage += 1;
                //			SetPageExpiredPage(1);
            case PAGE_STATUS_CHECKSUM_ERROR:
                //                CheckSumError += 1;
                //                SetCheckSumError(1);
            case PAGE_STATUS_CORRUPT:
            case PAGE_STATUS_INITIALIZED:
            case PAGE_STATUS_INACTIVE:
            {
                // Page is all zero.
                // ErasePage will increment Erase counter.
                PAGE_STATUS retCode;
                retCode = ErasePage(pageCount);
                if (retCode) return retCode;
                break;
            }
        }
        if (firstPage == 0) firstPage = pageCount;
    }
    if (GetPageCorruptStatus() & (firstPage == 0))
        return (6);
    if (expiredPage == NUM_DATA_EE_PAGES)
        return (2);
    if (CheckSumError == NUM_DATA_EE_PAGES)
        return (9); // Data verification failed in all pages,
    // number of failed words in high halfword
    // of dataEEFlags.

    // Count Active pages and identify Current page.
    ActivePage = 0;
    CurrentPage = 0;
    for (pageCount = 1; pageCount <= NUM_DATA_EE_PAGES; pageCount++) { // Ensure that Expired page is Not counted as Active.
        if (GetPageStatus(pageCount, STATUS_EXPIRED) == PAGE_NOT_EXPIRED) {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("%d - PAGE_NOT_EXPIRED\r\n", pageCount);
#endif

            if (GetPageStatus(pageCount, STATUS_ACTIVE) == PAGE_ACTIVE) {
                ActivePage++; // Count Active pages
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("%d - PAGE_ACTIVE\r\n", pageCount);
#endif

                if (GetPageStatus(pageCount, STATUS_CURRENT) == PAGE_CURRENT) {
                    CurrentPage = pageCount;
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%d - PAGE_CURRENT\r\n", pageCount);
#endif
                }
            }
        }
    }
    if (CheckSumError)
        pageSts = PAGE_STATUS_CHECKSUM_ERROR; // Checksum verification failed in some pages,
        // number of failed words in high halfword of
    else // dataEEFlags
        pageSts = PAGE_STATUS_VALID;

    // If no active pages found, initialize page 1
    if (ActivePage == 0) {
        // Initialize first page, Active & Current, keep Erase count.
        NVM_WordWrite(0xFFFD0FFF, EEDATA_PAGE_ADDR(firstPage - 1));
        while (NVM_IsBusy());

        CurrentPage = firstPage;
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("No active pages found, initialize page 1: %d\r\n", CurrentPage);
#endif

        ActivePage = 1;
        AddrIndex = 0;
        return (uint32_t) pageSts;
    }// If Full active pages, erase the page after the current page
    else if (ActivePage == NUM_DATA_EE_PAGES && NUM_DATA_EE_PAGES > 2) {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("Full active pages, erase the page %d\r\n", (CurrentPage % NUM_DATA_EE_PAGES) + 1);
#endif

        ErasePage((CurrentPage % NUM_DATA_EE_PAGES) + 1); // Erase the page after the current page
        AddrIndex = 0;
        AddrIndex = GetNextAvailCount();
        if (AddrIndex == 0xFFFF) PackEE(); // current Page is full

        return (uint32_t) pageSts;
    }// If some active pages, do nothing
    else if (ActivePage > 0) { // Find index to free area in CurrentPage.
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_MESSAGE("Some active pages, do nothing\r\n");
#endif

        AddrIndex = 0;
        AddrIndex = GetNextAvailCount();
        if (AddrIndex == 0xFFFF) pageSts = PackEE(); // Check if CurrentPage is full.

        if (CurrentPage == 0) CurrentPage = firstPage;

        return (uint32_t) pageSts;
    } else {
        SetPageCorruptStatus(1);
        return (6);
    }
}

/****************************************************************************
 * Function:        DataEEWrite
 *
 * PreCondition:    None
 *
 * Input:           Data EE address and data
 *
 * Output:          Check the dataEEFlags for the error status.
 *                  value 0 for success.
 *                  Value 2 for expired page.
 *                  Value 4 for pack skipped.
 *                  Value 5 for Illegal address.
 *                  Value 6 for page corrupt status.
 *                  Value 7 for write error.
 *                  Value 8 for Low voltage operation.
 *
 * Side Effects:    Data EE flags may be updated. CPU stall occurs for flash
 *                  programming. Pack may be generated.
 *
 * Overview:        This routine verifies the address is valid. If not, the Illegal Address
 *                  flag is set and an error code is returned. It then finds the active page.
 *                  If an active page can not be found, the Page Corrupt status bit is set
 *                  and an error code is returned. A read is performed, if the data was not
 *                  changed, the function exits. If the last location is programmed, the Pack
 *                  Skipped error flag is set (one location should always be available). The
 *                  data EE information address and data is programmed and verified. The data
 *                  checksum is written along with the address. 10 LSBits are allocated for
 *                  address and 6 bits are allotted for checksum. If the verify fails,
 *                  the Write Error flag is set. If the write went into the last location
 *                  of the page, pack is called. This function can be called by the user.
 *
 * Note:            This is a public function.
 *
 * Revision:
 * Arne Bergseth    Active page and Current page is not searched here,
 *                  use module static values,
 *                  maintained when a page status change occur.
 *****************************************************************************/
uint32_t DataEEWrite(uint32_t data, uint32_t addr) {
    uint32_t nextAddLoc;
    uint32_t addLoc;
    uint32_t nextDataLoc;
    uint32_t addCheckSum;
    uint32_t addrRead, dataRead;
    uint32_t retCode;

    // Check addr argument.
    if (addr >= DATA_EE_SIZE) {
        SetPageIllegalAddress(1);
        return (5);
    }

    // Do not write data if it did not change
    retCode = DataEERead(&dataRead, addr);
    if ((retCode == 0) && (dataRead == data)) // && (dataEEFlags.addrNotFound == 0))
    {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("%s: Data is Found and not change\r\n", __func__);
#endif

        return (0); // Data is Found and Equal.
    } else if (retCode > 1) {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("%s: error condition %d\r\n", __func__, retCode);
#endif

        return (retCode); // error condition
    }

    // Data is not found or not equal.
    // Use next available index.
    //    /* Mysil    AddrIndex = GetNextAvailCount();	/* May be redundant if AddrIndex and LowerAddress are kept current. */
    if (AddrIndex == 0xFFFF) // Page Full
    {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("%s: Page Full\r\n", __func__);
#endif

        SetPagePackSkipped(1);
        return (4); // Error - Number of writes exceeds page size
    }
    addLoc = (uint32_t) EEDATA_PAGE_ADDR(CurrentPage - 1) + 16;
    nextAddLoc = addLoc + AddrIndex;
    addCheckSum = (uint32_t) EmulationCheckSum(data);
    addCheckSum = addCheckSum << 0xA;
    dataEEFlags.addrNotFound = 0; // Address has been determined.
    if (LowerAddress == 0) {
        addr = ((addCheckSum | addr) << 16) | 0xFFFF;
        NVM_WordWrite(addr, nextAddLoc); // Writing address to the location
        while (NVM_IsBusy());

        nextDataLoc = (AddrIndex * 2) + DATA_OFFSET + addLoc;
        NVM_WordWrite(data, nextDataLoc); // Writing data to the location
        while (NVM_IsBusy());

#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("%s: Save1: nextAddLoc = %x, nextDataLoc = %x\r\n", __func__, nextAddLoc, nextDataLoc);
#endif

        // Check whether data and address are written correctly.
        NVM_Read(&addrRead, sizeof (addrRead), nextAddLoc);
        NVM_Read(&dataRead, sizeof (dataRead), nextDataLoc);

        if ((addr != addrRead) || (data != dataRead)) {
            SetPageWriteError(1);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("%s: Save not match 1\r\n", __func__);
#endif

            return (7); // Error - RAM does not match PM
        }
        LowerAddress = 1;
    } else if (LowerAddress == 1) {
        addr = addCheckSum | addr | 0xFFFF0000;
        NVM_WordWrite(addr, nextAddLoc); // Writing address to the location
        while (NVM_IsBusy());

        nextDataLoc = (AddrIndex * 2) + DATA_OFFSET + 4 + addLoc;
        NVM_WordWrite(data, nextDataLoc); // Writing data to the location
        while (NVM_IsBusy());

#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("%s: Save2: nextAddLoc = %x, nextDataLoc = %x\r\n", __func__, nextAddLoc, nextDataLoc);
#endif

        NVM_Read(&addrRead, sizeof (addrRead), nextAddLoc);
        NVM_Read(&dataRead, sizeof (dataRead), nextDataLoc);

        // Check whether data and address are written correctly.
        if (((addr << 16) != ((addrRead)) << 16) || (data != (dataRead))) {
            SetPageWriteError(1);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("%s: Save not match 2\r\n", __func__);
#endif

            return (7); // Error - RAM does not match PM
        }

        //  Check if page is full,  if (LowerAddress == 1)

        // If only one page, contents cannot be packed, and no more data can be stored.
        if (((AddrIndex + 4) == DATA_OFFSET) && (NUM_DATA_EE_PAGES == 1)) {
            SetPageCorruptStatus(1);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("%s: One page, no more data can be stored.\r\n", __func__);
#endif

            return (6);
        }// With only 2 pages, the contents must be packed.
        else if (((AddrIndex + 4) == DATA_OFFSET) && (NUM_DATA_EE_PAGES == 2)) {
            retCode = PackEE();
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("%s: 2 pages, PackEE() Error: %d\r\n", __func__, retCode);
#endif

            if (retCode) return (retCode);
        } else if (((AddrIndex + 4) == DATA_OFFSET) && (ActivePage == 1)) {
            // Mark the page as not_current and active
            NVM_WordWrite(0xFFF5FFFF, addLoc - 16);
            while (NVM_IsBusy());

            ActivePage++;

            // Mark the next page as current and active.
            NVM_WordWrite(0xFFFDFFFF, EEDATA_PAGE_ADDR(CurrentPage % NUM_DATA_EE_PAGES));
            while (NVM_IsBusy());

            CurrentPage = (CurrentPage % NUM_DATA_EE_PAGES) + 1;
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("%s: Set next current page, CurrentPage: %d\r\n", __func__, CurrentPage);
#endif

            AddrIndex = 0;
            LowerAddress = 0;
        } else if (((AddrIndex + 4) == DATA_OFFSET) && (ActivePage == 2)) // Both active pages are full then pack the page.
        {
            retCode = PackEE();
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("%s: Both active pages, PackEE() Error: %d\r\n", __func__, retCode);
#endif

            if (retCode) return (retCode);
        } else {
            AddrIndex += 4; // Next address.
            LowerAddress = 0;
        }
    }
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
    SYS_CONSOLE_PRINT("%s: OKKKKKK\r\n", __func__);
#endif

    return (0);
}

/****************************************************************************
 * Function:        DataEERead
 *
 * PreCondition:    None
 *
 * Input:           Read pointer and Data EE address
 *
 * Output:          Check the dataEEFlags for the error status.
 *                  value 0 for success.
 *                  Value 1 for address not found.
 *                  Value 5 for Illegal address.
 *                  Value 6 for page corrupt status.
 *
 * Side Effects:    Data EE flags may be updated.
 *
 * Overview:        This routine verifies whether the address is valid. If not, the Illegal Address
 *                  flag is set and 0 is returned. It then finds the active page. If an
 *                  active page can not be found, the Page Corrupt status bit is set and
 *                  0 is returned. A reverse search of the active page attempts to find
 *                  the matching address in the program memory. If a match is found,
 *                  the corresponding data EEPROM data is returned, otherwise 0
 *                  is returned. This function can be called by the user.
 *
 * Note:            This is a public function.
 *****************************************************************************/
uint32_t DataEERead(uint32_t *data, uint32_t addr) {
    uint32_t addLoc;
    uint32_t dataLoc;
    int32_t addrIndex;
    uint32_t addrRead;

    if (addr >= DATA_EE_SIZE) {
        SetPageIllegalAddress(1);
        return (5);
    }

    if (ActivePage == 0) {
        SetPageCorruptStatus(1);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("%s: ActivePage = 0, try init\r\n", __func__);
#endif

        return (6);
    }
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
    SYS_CONSOLE_PRINT("%s: CurrentPage = %d\r\n", __func__, CurrentPage);
#endif

    addLoc = (uint32_t) EEDATA_PAGE_ADDR(CurrentPage - 1) + 16;
    if (AddrIndex == 0xFFFF)
        addrIndex = DATA_OFFSET - 4; // Start at top of Page.
    else if (LowerAddress == 0)
        addrIndex = AddrIndex - 4;
    else
        addrIndex = AddrIndex; // Start at Current page stack pointer.

    //    uint32_t word;
    //    for (int i = 0; i < NUMBER_OF_INSTRUCTIONS_IN_PAGE; i++) {
    //        NVM_Read(&word, sizeof (word), EEDATA_PAGE_ADDR(CurrentPage - 1) + (i * 4));
    //        if (word != 0xFFFFFFFF) {
    //            SYS_CONSOLE_PRINT("CurrentPage: %d, i:%d, word: %x\r\n ", CurrentPage, i, word);
    //        }
    //    }

    while (addrIndex >= 0) // Scan the current page.
    {
        NVM_Read(&addrRead, sizeof (addrRead), (addrIndex + addLoc));
        //        1D0E8000-1D0FBFFF, 1D1E8000-1D1FBFFF
        //        if (addrRead != 0xFFFFFFFF)
        //            SYS_CONSOLE_PRINT("addrRead %x\r\n", addrRead);

        if ((addrRead & 0x03FF) == addr) {
            if ((addrRead & 0xFFFF) != 0xFFFF) {
                dataLoc = (addrIndex * 2) + DATA_OFFSET + 4 + addLoc;
                NVM_Read(data, sizeof (*data), dataLoc);
                if (((addrRead & 0xFC00) >> 0xA) == EmulationCheckSum(*data)) {
                    SetaddrNotFound(0);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: Address already exists 1\r\n", __func__);
#endif

                    return (0); // Success
                } else {
                    SetPageCorruptStatus(1);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: Address already exists but checksum error 1\r\n", __func__);
#endif

                    return (6);
                }
            }
        }
        if ((addrRead >> 16 & 0x3FF) == addr) {
            if ((addrRead >> 16 & 0xFFFF) != 0xFFFF) {
                dataLoc = (addrIndex * 2) + DATA_OFFSET + addLoc;
                NVM_Read(data, sizeof (*data), dataLoc);
                if (((addrRead & 0xFC000000) >> 0x1A) == EmulationCheckSum(*data)) {
                    SetaddrNotFound(0);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: Address already exists 2\r\n", __func__);
#endif

                    return 0; // Success
                } else {
                    SetPageCorruptStatus(1);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: Address already exists but checksum error 2\r\n", __func__);
#endif

                    return (6);
                }
            }
        }
        addrIndex -= 4;
    }

    if ((addrIndex < 0) && (ActivePage == 2)) {
        addLoc = (uint32_t) EEDATA_PAGE_ADDR(PrevPage(CurrentPage) - 1) + 16; // go to the previous page starting
    }

    if (ActivePage == 2) {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("%s: Goto prev page = %d\r\n", __func__, PrevPage(CurrentPage));
#endif

        addrIndex = DATA_OFFSET - 4; // go to the end of the page where address is stored.
        do // scan the second active page.
        {
            NVM_Read(&addrRead, sizeof (addrRead), (addrIndex + addLoc));
            if ((addrRead & 0x3FF) == addr) {
                dataLoc = (addrIndex * 2) + DATA_OFFSET + 4 + addLoc;
                NVM_Read(data, sizeof (*data), dataLoc);
                if (((addrRead & 0xFC00) >> 0xA) == EmulationCheckSum(*data)) {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: Address already exists 3\r\n", __func__);
#endif

                    return 0; // Success
                } else {
                    SetPageCorruptStatus(1);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: Address already exists but checksum error 3\r\n", __func__);
#endif

                    return (6);
                }
            } else if (((addrRead >> 16) & 0x3FF) == addr) {
                dataLoc = (addrIndex * 2) + DATA_OFFSET + addLoc;
                NVM_Read(data, sizeof (*data), dataLoc);
                if (((addrRead & 0xFC000000) >> 0x1A) == EmulationCheckSum(*data)) {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: Address already exists 4\r\n", __func__);
#endif

                    return 0; // Success
                } else {
                    SetPageCorruptStatus(1);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: Address already exists but checksum error 4\r\n", __func__);
#endif

                    return (6);
                }
            }
            addrIndex -= 4;
        } while (addrIndex >= 0);
    }
    if (addrIndex < 0) {
        SetaddrNotFound(1);
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
        SYS_CONSOLE_PRINT("%s: Address not found\r\n", __func__);
#endif

        return (1);
    }
    return 0;
}

/****************************************************************************
 * Function:        PackEE
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Check the dataEEFlags for the error status.
 *                  value 0 for success.
 *                  Value 6 for page corrupt status.
 *                  Value 7 for write error.
 *                  Value 8 for Low voltage operation.
 *
 * Side Effects:    Generates CPU stall during program/erase operations
 *                  Data EE flags may be updated
 *
 * Overview:        This routine finds the active page and an unexpired packed page. The most
 *                  recent data EEPROM values are located for each address will be read and
 *                  written into pack page. Page status is read from active
 *                  page and erase/write count is incremented if page 0 is packed. After all
 *                  information is programmed and verified, the current page is erased. The
 *                  packed page becomes the current page. This function can be called at any-
 *                  time by the user to schedule the CPU stall.
 *
 * Note:            This is a public function.
 *****************************************************************************/
uint32_t PackEE(void) {
    uint32_t data;
    uint32_t addr = 0;
    uint32_t addrWrite;
    uint32_t addrLoc;
    uint32_t dataLoc;
    uint32_t addrIndex = 0;
    uint32_t lowerAddr = 0;
    uint32_t addCheckSum;
    uint32_t nextPage = 0;
    uint32_t addrRead, dataRead;

#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
    SYS_CONSOLE_MESSAGE("\r\n\t_________________________\r\n");
    SYS_CONSOLE_PRINT("%s: Go\r\n", __func__);
    int pageCount;
    //    int activePage = 0;
    //    int currentPage = 0;

    // Check the active page.
    for (pageCount = 1; pageCount <= NUM_DATA_EE_PAGES; pageCount++)
        // Ensure that Expired page is Not counted as Active.
    {
        if (GetPageStatus(pageCount, STATUS_EXPIRED) == PAGE_NOT_EXPIRED) {
            SYS_CONSOLE_PRINT("%d - PAGE_NOT_EXPIRED\r\n", pageCount);
            if (GetPageStatus(pageCount, STATUS_ACTIVE) == PAGE_ACTIVE) {
                SYS_CONSOLE_PRINT("%d - PAGE_ACTIVE\r\n", pageCount);
                if (GetPageStatus(pageCount, STATUS_CURRENT) == PAGE_CURRENT) {
                    SYS_CONSOLE_PRINT("%d - PAGE_CURRENT\r\n", pageCount);
                }
            }
        }
    }
#endif


    if (AddrIndex == 0xFFFF) AddrIndex = DATA_OFFSET - 4;

    if (ActivePage == 1 && NUM_DATA_EE_PAGES > 2) {
        ;
    }// No action needed
    else if ((ActivePage == 2 && NUM_DATA_EE_PAGES > 2) || (ActivePage == 1 && NUM_DATA_EE_PAGES == 2)) {
        uint32_t status = 0;
        uint32_t packPage = (CurrentPage % NUM_DATA_EE_PAGES) + 1; // next page
        addrLoc = (uint32_t) EEDATA_PAGE_ADDR(packPage - 1) + 16; // get address location next page
        dataLoc = addrLoc + DATA_OFFSET; // data location
        do {
            //        addr = addCount;
            status = DataEERead(&data, addr);
            if (status == 1) { // addr not found, not copy
                addr++;
                continue;
            } else if (status > 1) { // read error
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("%s: Read addr error, addr = %x\r\n", __func__, addr);
#endif

                SetPageCorruptStatus(1);
                return (6);
            }
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("%s: Addr = %x founded -> copy\r\n", __func__, addr);
#endif

            addCheckSum = (uint32_t) EmulationCheckSum(data);
            addrWrite = addCheckSum << 0xA | addr;
            if (lowerAddr == 0) {
                addrWrite = addrWrite << 16 | 0xFFFF;
                NVM_WordWrite(addrWrite, addrLoc);
                while (NVM_IsBusy());
                NVM_WordWrite(data, dataLoc);
                while (NVM_IsBusy());

                // Check whether data and address are written correctly.
                NVM_Read(&addrRead, sizeof (addrRead), addrLoc);
                NVM_Read(&dataRead, sizeof (dataRead), dataLoc);

                if ((addrWrite != addrRead) || (data != dataRead)) {
                    //                if ((addrWrite != (*(int *) (addrLoc))) || (data != (*(int *) (dataLoc)))) {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: Save not match\r\n", __func__);
#endif

                    SetPageWriteError(1);
                    return (7); // Error - RAM does not match PM
                }
                lowerAddr = 1;
                dataLoc += 4;
            } else if (lowerAddr == 1) {
                addrWrite = addrWrite | 0xFFFF0000;
                NVM_WordWrite(addrWrite, addrLoc);
                while (NVM_IsBusy());
                NVM_WordWrite(data, dataLoc);
                while (NVM_IsBusy());


                // Check whether data and address are written correctly.
                NVM_Read(&addrRead, sizeof (addrRead), addrLoc);
                NVM_Read(&dataRead, sizeof (dataRead), dataLoc);

                if (((addrWrite << 16) != (addrRead << 16)) || (data != dataRead)) {
                    //                if (((addrWrite << 16) != ((*(int *) (addrLoc)) << 16)) || (data != (*(int *) (dataLoc)))) {
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
                    SYS_CONSOLE_PRINT("%s: Save not match\r\n", __func__);
#endif

                    SetPageWriteError(1);
                    return (7); // Error - RAM does not match PM
                }
                lowerAddr = 0;
                addrIndex += 4;
                addrLoc += 4;
                dataLoc += 4;
            }
            addr++;

        } while (addr < DATA_EE_SIZE); // (addCount < DATA_EE_SIZE);


        if (addrIndex < DATA_OFFSET) { // Space still available in packPage
            // Mark the packed page as Active and Current.
            NVM_WordWrite(0xFFFDFFFF, EEDATA_PAGE_ADDR(packPage - 1));
            while (NVM_IsBusy());
            status = ErasePage(CurrentPage); // Erase the old Current page

            if (NUM_DATA_EE_PAGES > 2 && ActivePage == 2) // Erase the Previous page
            {
                status = ErasePage(PrevPage(CurrentPage));
                ActivePage--;
            }
            CurrentPage = packPage; // Packed page is now Current.
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("%s: Space still available, Set current page: %d\r\n", __func__, CurrentPage);
#endif

        } else { // Pack page is filled. Mark as Active and Not Current.
            NVM_WordWrite(0xFFF5FFFF, EEDATA_PAGE_ADDR(packPage - 1)); // mark the packed page as active and not current.
            while (NVM_IsBusy());
            status = ErasePage(CurrentPage); // Erase the CurrentPage
            // Erase also Previous page if active.
            if (NUM_DATA_EE_PAGES > 2 && ActivePage == 2) {
                status = ErasePage(PrevPage(CurrentPage));
                ActivePage--;
            }
            // Mark the next page as Current and Active.
            nextPage = (packPage % NUM_DATA_EE_PAGES) + 1;
            NVM_WordWrite(0xFFFDFFFF, EEDATA_PAGE_ADDR(packPage - 1));
            while (NVM_IsBusy());

            //            if (!retCode) {
            ActivePage++;
            CurrentPage = nextPage; // Next page is now Current.
#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("%s: Pack page is filled, Set current page: %d\r\n", __func__, CurrentPage);
#endif

            //            } else {
            //            ActivePage = 1;
            //            CurrentPage = packPage; // Packed page is now Current.
            //            }
        }
        AddrIndex = 0; // Reset index.
        AddrIndex = GetNextAvailCount(); // Search for erased area.

        //		if (status)			// Check status from ErasePage()
        //			return (status);
    } else {

        SetPagePackBeforeInit(1); // Something wrong, should not be here.
    }

#if defined(DEBUG_MODULE_DEE) || defined(DEBUG_MODULE_ALL)
    // Check the active page.
    for (pageCount = 1; pageCount <= NUM_DATA_EE_PAGES; pageCount++)
        // Ensure that Expired page is Not counted as Active.
    {
        if (GetPageStatus(pageCount, STATUS_EXPIRED) == PAGE_NOT_EXPIRED) {
            SYS_CONSOLE_PRINT("%d - PAGE_NOT_EXPIRED\r\n", pageCount);
            if (GetPageStatus(pageCount, STATUS_ACTIVE) == PAGE_ACTIVE) {
                SYS_CONSOLE_PRINT("%d - PAGE_ACTIVE\r\n", pageCount);
                if (GetPageStatus(pageCount, STATUS_CURRENT) == PAGE_CURRENT) {
                    SYS_CONSOLE_PRINT("%d - PAGE_CURRENT\r\n", pageCount);
                }
            }
        }
    }

    uint32_t v1, v2;
    DataEERead(&v1, 0);
    DataEERead(&v2, 1);
    SYS_CONSOLE_MESSAGE("\r\n ---------------- \r\n");
    SYS_CONSOLE_PRINT("load val1 = %u,  val2 = %u\r\n", v1, v2);
    SYS_CONSOLE_MESSAGE("\r\n ---------------- \r\n");

    SYS_CONSOLE_MESSAGE("\r\n\t_____________PACKEE end____________\r\n");
#endif

    return (0);
}

/****************************************************************************
 * Function:        DataEEWriteArray
 *
 * PreCondition:    None
 *
 * Input:           char array pointer, address to be started and the size of array
 *
 * Output:          Check the dataEEFlags for the error status.
 *                  value 0 for success.
 *                  Value 2 for expired page.
 *                  Value 4 for pack skipped.
 *                  Value 5 for Illegal address.
 *                  Value 6 for page corrupt status.
 *                  Value 7 for write error.
 *                  Value 8 for Low voltage operation.
 *
 * Side Effects:    Data EE flags may be updated. CPU stall occurs for flash
 *                  programming. Pack may be generated.
 *
 * Overview:        This routine will write a char array of data with a given
 *                  starting address upto the array size specified by the user.
 *                  Use DataEEWriteArray function to read the data written using this function
 *                  This is solely designed to write char array.
 *
 * Note:            This is a public function.
 *****************************************************************************/
uint32_t DataEEWriteArray(uint8_t *data, uint32_t addr, uint32_t size) {
    int i;
    int numberOfWords;
    uint32_t status = 0;
    uint32_t writeData;
    uint8_t *tempData;

    if ((size % 4)) {
        numberOfWords = (size / 4) + 1;
    } else {
        numberOfWords = (size / 4);
    }
    tempData = data;

    for (i = 0; i < numberOfWords; i++) {
        writeData = 0xFFFFFFFF;
        int bytesLeft = size - i * 4;

        if (bytesLeft >= 4) {
            writeData = ((uint32_t) tempData[0]) |
                    ((uint32_t) tempData[1] << 8) |
                    ((uint32_t) tempData[2] << 16) |
                    ((uint32_t) tempData[3] << 24);
        } else {
            for (int j = 0; j < bytesLeft; j++) {
                writeData &= ~(0xFF << (8 * j));
                writeData |= ((uint32_t) tempData[j]) << (8 * j);
            }
        }

        if ((status = DataEEWrite(writeData, addr)) > 0)
            return status;

        tempData += 4;
        addr++;
    }
    return (0);
}

/****************************************************************************
 * Function:        DataEEReadArray
 *
 * PreCondition:    None
 *
 * Input:           data pointer, address to be started and the size of array
 *
 * Output:          Check the dataEEFlags for the error status.
 *                  value 0 for success.
 *                  Value 1 for address not found.
 *                  Value 5 for Illegal address.
 *                  Value 6 for page corrupt status.
 *
 * Side Effects:    Data EE flags may be updated. Reading the data in between the
 *                  array address will cause incorrect data return.
 *
 * Overview:        This function will read a byte array of values starting from addr
 *                  and will copy to the array pointer "data". User should read the
 *                  data from the starting address of the array.
 *
 * Note:            This is a public function.
 * Revision:
 *	Arne Bergseth   Modified to avoid output buffer overflow
 *****************************************************************************/
uint32_t DataEEReadArray(uint8_t *data, uint32_t addr, uint32_t size) {
    int i;
    uint32_t status = 0;
    uint32_t numberOfWords, numberOfBytes;
    uint32_t readData;
    uint32_t count = 0;

    numberOfWords = (size / 4);

    for (i = 0; i < numberOfWords; i++) // Transfer whole 32bit words
    {
        if ((status = DataEERead(&readData, addr)) > 0)
            return status;

        *(data + count++) = (readData & 0xFF);
        *(data + count++) = (readData & 0xFF00) >> 8;
        *(data + count++) = (readData & 0xFF0000) >> 16;
        *(data + count++) = (readData & 0xFF000000) >> 24;
        addr++;
    }
    if ((numberOfBytes = size % 4)) // Transfer remaining bytes.
    {
        if ((status = DataEERead(&readData, addr)) > 0)
            return status;

        *(data + count++) = (readData & 0xFF);
        if (numberOfBytes > 1)
            *(data + count++) = (readData & 0xFF00) >> 8;
        if (numberOfBytes > 2)
            *(data + count++) = (readData & 0xFF0000) >> 16;
        if (numberOfBytes > 3)
            *(data + count++) = (readData & 0xFF000000) >> 24;
    }
    return (0);
}

