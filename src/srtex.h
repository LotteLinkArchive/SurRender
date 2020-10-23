#ifndef SURTXD_HEADER_FILE
#define SURTXD_HEADER_FILE

/* Defines the structure of an SRT texture header. */
struct SRTHeader {
	/* The width and height of the SRT texture */
	I32 width;
	I32 height;

	/* The amount of byte-large channels (as returned by STBI) */
	I32 stbi_type;

	/* The data length in bytes */
	I32 data_length;

	/* After the header is written to the file, the entire rest of the file should be of size data_length and
	 * contain the required image data.
	 */
};

#endif