/******************************************************************************
 *
 *
 * Copyright 2011, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/
//#include <gtest/gtest.h>


#include <qcc/platform.h>

#include <qcc/Crypto.h>
#include <qcc/Debug.h>
#include <qcc/FileStream.h>
#include <qcc/KeyBlob.h>
#include <qcc/Pipe.h>
#include <qcc/StringUtil.h>
#include <qcc/Util.h>
#include <qcc/GUID.h>
#include <qcc/time.h>

#include <alljoyn/version.h>
#include "KeyStore.h"

#include <Status.h>

//#include <gtest/gtest.h>

using namespace qcc;
using namespace std;
using namespace ajn;

static const char testData[] = "This is the message that we are going to encrypt and then decrypt and verify";

void foo()
{
#pragma pack(show)

	int iSize = sizeof(KeyStore);  // the compiler says KeyStore is
    {
        KeyStore* keyStore = new KeyStore("keystore_test");

//        keyStore.Init(NULL, true);
//        keyStore.Clear();
//
//        key.Rand(Crypto_AES::AES128_SIZE, KeyBlob::AES);
//        keyStore.AddKey(guid1, key);
//        key.Rand(620, KeyBlob::GENERIC);
//        keyStore.AddKey(guid2, key);
//
//        status = keyStore.Store();
//        ASSERT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status) << " Failed to store keystore";
        delete keyStore;
    }

}


/** Main entry point */
int main(int argc, char**argv, char**envArg)
{
    int status = 0;
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("\n Running alljoyn_core unit test\n");

    foo();



//    testing::InitGoogleTest(&argc, argv);
//    status = RUN_ALL_TESTS();

    printf("%s exiting with status %d \n", argv[0], status);

    return (int) status;
}
