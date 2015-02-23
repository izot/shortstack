/*
 * IzoT ShortStack for Raspberry Pi Example code.
 *
 * io.h declares the interface for functions in the io folder. These provide
 * utilities for handling general-purpose I/O.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */
#ifndef RPI_EXAMPLE_IO_DEFINED
#	define RPI_EXAMPLE_IO_DEFINED

extern int GpioOpen(
    int port, mode_t mode, const char* dir, const char* trigger
);

extern int GpioClose(int port, int handle);


#endif /* RPI_EXAMPLE_IO_DEFINED */
