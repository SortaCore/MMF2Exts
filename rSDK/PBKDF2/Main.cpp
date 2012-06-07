// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================
#include "common.h"
std::string LastOutput="";
// ============================================================================
//
// CONDITIONS
// 
// ============================================================================

// ============================================================================
//
// ACTIONS
// 
// ============================================================================

ACTION(
	/* ID */			0,
	/* Name */			"Hash with PBKDF2",
	/* Flags */			0,
	/* Params */		(2|EXPPARAM_STRING|EXPPARAM_STRING|EXPPARAM_STRING)
) {
    CkCrypt2 crypt;
    bool success;
    success = crypt.UnlockComponent("Anything for 30-day trial");
    if (success != true) {
        LastOutput += crypt.lastErrorText();
        return;
    }

    const char * hexKey;
    //  http://www.di-mgt.com.au/cryptoKDFs.html#examplespbkdf

    const char * pw;
    pw = "password";
    const char * pwCharset;
    pwCharset = "ansi";
    //  Hash algorithms may be: sha1, md2, md5, etc.
    const char * hashAlg;
    hashAlg = "sha1";
    //  The salt should be 8 bytes:
    const char * saltHex;
    saltHex = "78578E5A5D63CB06";
    long iterationCount;
    iterationCount = 2048;
    //  Derive a 192-bit key from the password.
    long outputBitLen;
    outputBitLen = 192;

    //  The derived key is returned as a hex or base64 encoded string.
    //  (Note: The salt argument must be a string that also uses
    //  the same encoding.)
    const char * enc;
    enc = "hex";

	hexKey = crypt.pbkdf2(pw,pwCharset,hashAlg,saltHex,iterationCount,outputBitLen,enc);

    hexKey;

    //  The output should have this value:
    //  BFDE6BE94DF7E11DD409BCE20A0255EC327CB936FFE93643

	rdPtr->rRd->GenerateEvent(0);

}

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================

EXPRESSION(
	/* ID */			0,
	/* Name */			"LastFixedValue(",
	/* Flags */			0,
	/* Params */		(0)
) {
	return 0;
}
