/******************************************************************************
 *
 * Copyright 2011-2012, Qualcomm Innovation Center, Inc.
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
 *
 *****************************************************************************/

#include "MsgArg.h"

#include <TypeCoercerFactory.h>
#include <StrictTypeCoercer.h>
#include <qcc/winrt/utility.h>
#include <qcc/Debug.h>
#include <SignatureUtils.h>
#include <ObjectReference.h>
#include <AllJoynException.h>

using namespace Windows::Foundation;
using namespace ajn;

#define QCC_MODULE "ALLJOYN"

#define ADD_SCRATCH(s) \
    { _msgScratch.push_back(s); \
    }

#define CLEAR_SCRATCH() \
    { std::list<void*>::const_iterator itr2; \
      for (itr2 = _msgScratch.begin(); itr2 != _msgScratch.end(); ++itr2) { \
          delete [] *itr2; \
      } \
      _msgScratch.clear(); \
    }

#define ADD_STRING_REF(s) \
    { _strRef.push_back(s); \
    }

#define CLEAR_STRING_REFS() \
    { _strRef.clear(); \
    }

namespace AllJoyn {

ITypeCoercer* typeCoercer = TypeCoercerFactory::GetTypeCoercer("strict");

MsgArg::MsgArg()
{
    ::QStatus status = ER_OK;

    while (true) {
        _MsgArg* ma = new _MsgArg();
        if (NULL == ma) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mMsgArg = new qcc::ManagedObj<_MsgArg>(ma);
        if (NULL == _mMsgArg) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _msgArg = &(**_mMsgArg);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

MsgArg::MsgArg(Platform::String ^ signature, const Platform::Array<Platform::Object ^> ^ args)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == args) {
            status = ER_BAD_ARG_2;
            break;
        }
        _MsgArg* ma = new _MsgArg();
        if (NULL == ma) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mMsgArg = new qcc::ManagedObj<_MsgArg>(ma);
        if (NULL == _mMsgArg) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _msgArg = &(**_mMsgArg);
        qcc::String strSignature = PlatformToMultibyteString(signature);
        size_t sigLen = (strSignature.c_str() ? strlen(strSignature.c_str()) : 0);
        if ((sigLen < 1) || (sigLen > 255)) {
            status = ER_BUS_BAD_SIGNATURE;
            break;
        } else {
            const char* signature = strSignature.c_str();
            int argIndex = 0;
            status = _msgArg->VBuildArgs(signature, sigLen, _msgArg, 1, args, argIndex, 0);
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

MsgArg::MsgArg(const ajn::MsgArg* msgArg)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == msgArg) {
            status = ER_BAD_ARG_1;
            break;
        }
        _MsgArg* ma = new _MsgArg();
        if (NULL == ma) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::MsgArg* destArg = ma;
        *destArg = *msgArg;
        _mMsgArg = new qcc::ManagedObj<_MsgArg>(ma);
        if (NULL == _mMsgArg) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _msgArg = &(**_mMsgArg);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

MsgArg::MsgArg(const qcc::ManagedObj<_MsgArg>* msgArg)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == msgArg) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mMsgArg = new qcc::ManagedObj<_MsgArg>(*msgArg);
        if (NULL == _mMsgArg) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _msgArg = &(**_mMsgArg);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

MsgArg::~MsgArg()
{
    if (NULL != _mMsgArg) {
        delete _mMsgArg;
        _mMsgArg = NULL;
        _msgArg = NULL;
    }
}

Platform::Object ^ MsgArg::Value::get()
{
    Platform::Object ^ val = _msgArg->_eventsAndProperties->Value;
    if (nullptr == val) {
        _msgArg->SetObject(this, false);
        val = _msgArg->_eventsAndProperties->Value;
    }
    return val;
}

Platform::Object ^ MsgArg::Key::get()
{
    Platform::Object ^ key = _msgArg->_eventsAndProperties->Key;
    if (nullptr == key && _msgArg->typeId == ajn::AllJoynTypeId::ALLJOYN_DICT_ENTRY) {
        _msgArg->SetObject(this, true);
        key = _msgArg->_eventsAndProperties->Key;
    }
    return key;
}

void MsgArg::SetTypeCoercionMode(Platform::String ^ mode)
{
    typeCoercer = TypeCoercerFactory::GetTypeCoercer(mode);
}

_MsgArg::_MsgArg()
    : ajn::MsgArg()
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __MsgArg();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_MsgArg::~_MsgArg()
{
    _eventsAndProperties = nullptr;
    ClearObjectMap(NULL, &(this->_refMap));
    CLEAR_SCRATCH();
    CLEAR_STRING_REFS();
}

::QStatus _MsgArg::BuildArray(ajn::MsgArg* arry, const qcc::String elemSig, const Platform::Array<Platform::Object ^> ^ args, int32_t& argIndex)
{
    ::QStatus status = ER_OK;
    ajn::MsgArg* elements = NULL;

    switch (elemSig[0]) {
    case '*':
    case 'a':
    case 'v':
    case '(':
    case '{':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ objVariantArray = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_ARRAY, true);
            if (nullptr == objVariantArray) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            // MsgArg []
            Platform::IBoxArray<Platform::Object ^> ^ boxArray = dynamic_cast<Platform::IBoxArray<Platform::Object ^> ^>(objVariantArray);
            if (nullptr == boxArray) {
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::Array<Platform::Object ^> ^ objArray = boxArray->Value;
            if (nullptr == objArray) {
                status = ER_FAIL;
                break;
            }
            size_t numElements = objArray->Length;
            if (numElements < 1) {
                status = ER_BUS_BAD_VALUE;
                QCC_LogError(status, ("Wildcard element signature cannot be used with an empty array"));
                break;
            }
            // Allocate an array for the msgarg pointers
            ajn::MsgArg* nativeArgs = new ajn::MsgArg[numElements];
            if (NULL == nativeArgs) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            // Hold on to msgarg scratch to release later
            ADD_SCRATCH(nativeArgs);
            for (int i = 0; i < numElements; i++) {
                Platform::Object ^ obj = typeCoercer->Coerce(objArray[i], ajn::ALLJOYN_VARIANT, true);
                if (nullptr == obj) {
                    // Invalid conversion
                    status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                    break;
                }
                AllJoyn::MsgArg ^ msgarg = dynamic_cast<AllJoyn::MsgArg ^>(obj);
                if (nullptr == msgarg) {
                    status = ER_FAIL;
                    break;
                }
                AddObjectReference(NULL, msgarg, &(this->_refMap));
                // Store the pointer
                ajn::MsgArg* temp = msgarg->_msgArg;
                nativeArgs[i] = *temp;
            }
            AddObjectReference(NULL, objArray, &(this->_refMap));
            elements = nativeArgs;
            const qcc::String sig = elements[0].Signature();
            /*
             * Check elements all have same type as the first element.
             */
            for (size_t i = 1; i < numElements; i++) {
                if (!elements[i].HasSignature(sig.c_str())) {
                    status = ER_BUS_BAD_VALUE;
                    QCC_LogError(status, ("Array element[%d] does not have expected signature \"%s\"", i, sig.c_str()));
                    break;
                }
            }
            if (status == ER_OK) {
                status = arry->v_array.SetElements(sig.c_str(), numElements, elements);
            }
        }
    }
    break;

    case 'h':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_UINT64_ARRAY, true);
            if (nullptr == obj) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::IBoxArray<uint64> ^ boxArray = dynamic_cast<Platform::IBoxArray<uint64> ^>(obj);
            if (nullptr == boxArray) {
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::Array<uint64> ^ sArray = boxArray->Value;
            if (nullptr == sArray) {
                status = ER_FAIL;
                break;
            }
            size_t numElements = sArray->Length;
            if (numElements < 1) {
                status = ER_BUS_BAD_VALUE;
                break;
            }
            // Allocate an array for the msgarg pointers
            ajn::MsgArg* nativeArgs = new ajn::MsgArg[numElements];
            if (NULL == nativeArgs) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            // Hold on to msgarg scratch to release later
            ADD_SCRATCH(nativeArgs);
            for (int i = 0; i < numElements; i++) {
                Platform::Object ^ o = PropertyValue::CreateUInt64(sArray[i]);
                if (nullptr == o) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                Platform::Array<Platform::Object ^> ^ objArr = ref new Platform::Array<Platform::Object ^>(1);
                if (nullptr == objArr) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                objArr[0] = o;
                AllJoyn::MsgArg ^ msgarg = ref new AllJoyn::MsgArg("h", objArr);
                if (nullptr == msgarg) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                AddObjectReference(NULL, msgarg, &(this->_refMap));
                // Store the pointer
                ajn::MsgArg* temp = msgarg->_msgArg;
                nativeArgs[i] = *temp;
            }
            elements = nativeArgs;
            const qcc::String sig = elements[0].Signature();
            if (status == ER_OK) {
                status = arry->v_array.SetElements(sig.c_str(), numElements, elements);
            }
        }
    }
    break;

    case 'o':
    case 's':
    case 'g':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, (ajn::AllJoynTypeId)((elemSig[0] << 8) | 'a'), true);
            if (nullptr == obj) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::IBoxArray<Platform::String ^> ^ boxArray = dynamic_cast<Platform::IBoxArray<Platform::String ^> ^>(obj);
            if (nullptr == boxArray) {
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::Array<Platform::String ^> ^ sArray = boxArray->Value;
            if (nullptr == sArray) {
                status = ER_FAIL;
                break;
            }
            size_t numElements = sArray->Length;
            if (numElements < 1) {
                status = ER_BUS_BAD_VALUE;
                break;
            }
            // Allocate an array for the msgarg pointers
            ajn::MsgArg* nativeArgs = new ajn::MsgArg[numElements];
            if (NULL == nativeArgs) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            // Hold on to msgarg scratch to release later
            ADD_SCRATCH(nativeArgs);
            for (int i = 0; i < numElements; i++) {
                Platform::Array<Platform::Object ^> ^ objArr = ref new Platform::Array<Platform::Object ^>(1);
                if (nullptr == objArr) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                objArr[0] = sArray[i];
                AllJoyn::MsgArg ^ msgarg = ref new AllJoyn::MsgArg((elemSig[0] == 's') ? "s" : (elemSig[0] == 'o') ? "o" : "g", objArr);
                if (nullptr == msgarg) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                AddObjectReference(NULL, msgarg, &(this->_refMap));
                // Store the pointer
                ajn::MsgArg* temp = msgarg->_msgArg;
                nativeArgs[i] = *temp;
            }
            elements = nativeArgs;
            const qcc::String sig = elements[0].Signature();
            if (status == ER_OK) {
                status = arry->v_array.SetElements(sig.c_str(), numElements, elements);
            }
        }
    }
    break;

    case 'b':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_BOOLEAN_ARRAY, true);
            if (nullptr == obj) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::IBoxArray<Platform::Boolean> ^ boxArray = dynamic_cast<Platform::IBoxArray<Platform::Boolean> ^>(obj);
            if (nullptr != boxArray) {
                Platform::Array<Platform::Boolean> ^ objArray = boxArray->Value;
                if (nullptr == objArray || objArray->Length < 1) {
                    status = ER_FAIL;
                    break;
                }
                AddObjectReference(NULL, objArray, &(this->_refMap));
                arry->typeId = ALLJOYN_BOOLEAN_ARRAY;
                arry->v_scalarArray.v_bool = objArray->Data;
                arry->v_scalarArray.numElements = objArray->Length;
            } else {
                // Invalid conversion
                status = ER_FAIL;
                break;
            }
        }
    }
    break;

    case 'd':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_DOUBLE_ARRAY, true);
            if (nullptr == obj) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::IBoxArray<float64> ^ boxArray = dynamic_cast<Platform::IBoxArray<float64> ^>(obj);
            if (nullptr != boxArray) {
                Platform::Array<float64> ^ objArray = boxArray->Value;
                if (nullptr == objArray || objArray->Length < 1) {
                    status = ER_FAIL;
                    break;
                }
                AddObjectReference(NULL, objArray, &(this->_refMap));
                arry->typeId = ALLJOYN_DOUBLE_ARRAY;
                arry->v_scalarArray.v_double = objArray->Data;
                arry->v_scalarArray.numElements = objArray->Length;
            } else {
                // Invalid conversion
                status = ER_FAIL;
                break;
            }
        }
    }
    break;

    case 'i':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_INT32_ARRAY, true);
            if (nullptr == obj) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::IBoxArray<int32> ^ boxArray = dynamic_cast<Platform::IBoxArray<int32> ^>(obj);
            if (nullptr != boxArray) {
                Platform::Array<int32> ^ objArray = boxArray->Value;
                if (nullptr == objArray || objArray->Length < 1) {
                    status = ER_FAIL;
                    break;
                }
                AddObjectReference(NULL, objArray, &(this->_refMap));
                arry->typeId = ALLJOYN_INT32_ARRAY;
                arry->v_scalarArray.v_int32 = objArray->Data;
                arry->v_scalarArray.numElements = objArray->Length;
            } else {
                // Invalid conversion
                status = ER_FAIL;
                break;
            }
        }
    }
    break;

    case 'n':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_INT16_ARRAY, true);
            if (nullptr == obj) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::IBoxArray<int16> ^ boxArray = dynamic_cast<Platform::IBoxArray<int16> ^>(obj);
            if (nullptr != boxArray) {
                Platform::Array<int16> ^ objArray = boxArray->Value;
                if (nullptr == objArray || objArray->Length < 1) {
                    status = ER_FAIL;
                    break;
                }
                AddObjectReference(NULL, objArray, &(this->_refMap));
                arry->typeId = ALLJOYN_INT16_ARRAY;
                arry->v_scalarArray.v_int16 = objArray->Data;
                arry->v_scalarArray.numElements = objArray->Length;
            } else {
                // Invalid conversion
                status = ER_FAIL;
                break;
            }
        }
    }
    break;

    case 'q':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_UINT16_ARRAY, true);
            if (nullptr == obj) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::IBoxArray<uint16> ^ boxArray = dynamic_cast<Platform::IBoxArray<uint16> ^>(obj);
            if (nullptr != boxArray) {
                Platform::Array<uint16> ^ objArray = boxArray->Value;
                if (nullptr == objArray || objArray->Length < 1) {
                    status = ER_FAIL;
                    break;
                }
                AddObjectReference(NULL, objArray, &(this->_refMap));
                arry->typeId = ALLJOYN_UINT16_ARRAY;
                arry->v_scalarArray.v_uint16 = objArray->Data;
                arry->v_scalarArray.numElements = objArray->Length;
            } else {
                // Invalid conversion
                status = ER_FAIL;
                break;
            }
        }
    }
    break;

    case 't':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_UINT64_ARRAY, true);
            if (nullptr == obj) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::IBoxArray<uint64> ^ boxArray = dynamic_cast<Platform::IBoxArray<uint64> ^>(obj);
            if (nullptr != boxArray) {
                Platform::Array<uint64> ^ objArray = boxArray->Value;
                if (nullptr == objArray || objArray->Length < 1) {
                    status = ER_FAIL;
                    break;
                }
                AddObjectReference(NULL, objArray, &(this->_refMap));
                arry->typeId = ALLJOYN_UINT64_ARRAY;
                arry->v_scalarArray.v_uint64 = objArray->Data;
                arry->v_scalarArray.numElements = objArray->Length;
            } else {
                // Invalid conversion
                status = ER_FAIL;
                break;
            }
        }
    }
    break;

    case 'u':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_UINT32_ARRAY, true);
            if (nullptr == obj) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::IBoxArray<uint32> ^ boxArray = dynamic_cast<Platform::IBoxArray<uint32> ^>(obj);
            if (nullptr != boxArray) {
                Platform::Array<uint32> ^ objArray = boxArray->Value;
                if (nullptr == objArray || objArray->Length < 1) {
                    status = ER_FAIL;
                    break;
                }
                AddObjectReference(NULL, objArray, &(this->_refMap));
                arry->typeId = ALLJOYN_UINT32_ARRAY;
                arry->v_scalarArray.v_uint32 = objArray->Data;
                arry->v_scalarArray.numElements = objArray->Length;
            } else {
                // Invalid conversion
                status = ER_FAIL;
                break;
            }
        }
    }
    break;

    case 'x':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_INT64_ARRAY, true);
            if (nullptr == obj) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::IBoxArray<int64> ^ boxArray = dynamic_cast<Platform::IBoxArray<int64> ^>(obj);
            if (nullptr != boxArray) {
                Platform::Array<int64> ^ objArray = boxArray->Value;
                if (nullptr == objArray || objArray->Length < 1) {
                    status = ER_FAIL;
                    break;
                }
                AddObjectReference(NULL, objArray, &(this->_refMap));
                arry->typeId = ALLJOYN_INT64_ARRAY;
                arry->v_scalarArray.v_int64 = objArray->Data;
                arry->v_scalarArray.numElements = objArray->Length;
            } else {
                // Invalid conversion
                status = ER_FAIL;
                break;
            }
        }
    }
    break;

    case 'y':
    {
        Platform::Object ^ rawObj = args[argIndex++];
        if (nullptr != rawObj) {
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_BYTE_ARRAY, true);
            if (nullptr == obj) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::IBoxArray<uint8> ^ boxArray = dynamic_cast<Platform::IBoxArray<uint8> ^>(obj);
            if (nullptr != boxArray) {
                Platform::Array<uint8> ^ objArray = boxArray->Value;
                if (nullptr == objArray || objArray->Length < 1) {
                    status = ER_FAIL;
                    break;
                }
                AddObjectReference(NULL, objArray, &(this->_refMap));
                arry->typeId = ALLJOYN_BYTE_ARRAY;
                arry->v_scalarArray.v_byte = objArray->Data;
                arry->v_scalarArray.numElements = objArray->Length;
            } else {
                // Invalid conversion
                status = ER_FAIL;
                break;
            }
        }
    }
    break;

    default:
        status = ER_BUS_BAD_SIGNATURE;
        QCC_LogError(status, ("Invalid char '\\%d' in array element signature", elemSig[0]));
        break;
    }
    if (status != ER_OK) {
        arry->typeId = ALLJOYN_INVALID;
    }
    return status;
}

::QStatus _MsgArg::VBuildArgs(const char*& signature, uint32_t sigLen, ajn::MsgArg* arg, int32_t maxCompleteTypes, const Platform::Array<Platform::Object ^> ^ args, int32_t& argIndex, int32_t recursionLevel)
{
    ::QStatus status = ER_OK;
    size_t numArgs = 0;

    while (sigLen-- && argIndex < args->Length && maxCompleteTypes--) {
        switch (*signature++) {
        case '*':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_VARIANT, true);
            if (nullptr != obj) {
                AllJoyn::MsgArg ^ val = dynamic_cast<AllJoyn::MsgArg ^>(obj);
                ajn::MsgArg* v = NULL;
                if (nullptr != val && NULL != val->_msgArg) {
                    v = val->_msgArg;
                    AddObjectReference(NULL, obj, &(this->_refMap));
                } else {
                    // Invalid conversion
                    status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                    break;
                }
                if (v->typeId == ALLJOYN_ARRAY) {
                    status = arg->v_array.SetElements(v->v_array.GetElemSig(), v->v_array.GetNumElements(), (ajn::MsgArg*)v->v_array.GetElements());
                } else {
                    *arg = *v;
                }
            } else {
                Platform::Object ^ objVariantArray = typeCoercer->Coerce(args[argIndex - 1], ajn::ALLJOYN_ARRAY, true);
                if (nullptr == objVariantArray) {
                    // Invalid conversion
                    status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                    break;
                }
                --argIndex;
                const char* elemSig = signature - 1;
                arg->typeId = ALLJOYN_ARRAY;
                size_t elemSigLen = signature - elemSig;
                status = BuildArray(arg, qcc::String(elemSig, elemSigLen), args, argIndex);
                sigLen -= (elemSigLen - 1);
            }
        }
        break;

        case 'a':
        {
            const char* elemSig = signature;
            arg->typeId = ALLJOYN_ARRAY;
            if (*elemSig == '*') {
                ++signature;
            } else {
                status = SignatureUtils::ParseContainerSignature(*arg, signature);
            }
            if (status == ER_OK) {
                size_t elemSigLen = signature - elemSig;
                status = BuildArray(arg, qcc::String(elemSig, elemSigLen), args, argIndex);
                sigLen -= elemSigLen;
            } else {
                status = ER_BUS_NOT_A_COMPLETE_TYPE;
                QCC_LogError(status, ("Signature for array was not a complete type"));
                arg->typeId = ALLJOYN_INVALID;
            }
        }
        break;

        case 'b':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_BOOLEAN, true);
            Platform::IBox<Platform::Boolean> ^ t = dynamic_cast<Platform::IBox<Platform::Boolean> ^>(obj);
            if (nullptr != t) {
                Platform::Boolean param = t->Value;
                arg->typeId = ALLJOYN_BOOLEAN;
                arg->v_bool = param;
                AddObjectReference(NULL, obj, &(this->_refMap));
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 'd':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_DOUBLE, true);
            Platform::IBox<float64> ^ t = dynamic_cast<Platform::IBox<float64> ^>(obj);
            if (nullptr != t) {
                float64 param = t->Value;
                arg->typeId = ALLJOYN_DOUBLE;
                arg->v_double = param;
                AddObjectReference(NULL, obj, &(this->_refMap));
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 'e':
        {
            if ((argIndex + 1) < args->Length) {
                Platform::Object ^ objKey = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_VARIANT, true);
                AllJoyn::MsgArg ^ key = nullptr;
                if (nullptr != objKey) {
                    key = dynamic_cast<AllJoyn::MsgArg ^>(objKey);
                }
                ajn::MsgArg* k = NULL;
                if (nullptr != key || NULL == key->_msgArg) {
                    k = key->_msgArg;
                    AddObjectReference(NULL, objKey, &(this->_refMap));
                } else {
                    // Invalid conversion
                    status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                    break;
                }
                Platform::Object ^ objVal = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_VARIANT, true);
                AllJoyn::MsgArg ^ val = nullptr;
                if (nullptr != objVal) {
                    val = dynamic_cast<AllJoyn::MsgArg ^>(objVal);
                }
                ajn::MsgArg* v = NULL;
                if (nullptr != val && NULL != val->_msgArg) {
                    v = val->_msgArg;
                    AddObjectReference(NULL, objVal, &(this->_refMap));
                } else {
                    // Invalid conversion
                    status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                    break;
                }
                arg->typeId = ALLJOYN_DICT_ENTRY;
                arg->v_dictEntry.key = k;
                arg->v_dictEntry.val = v;
            } else {
                status = ER_BAD_ARG_COUNT;
                break;
            }
        }
        break;

        case 'g':
        {
            Platform::Object ^ rawObj = args[argIndex++];
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_SIGNATURE, true);
            Platform::String ^ t = dynamic_cast<Platform::String ^>(obj);
            if (nullptr != obj || nullptr == rawObj) {
                Platform::String ^ param = t;
                qcc::String strParam = PlatformToMultibyteString(param);
                if (nullptr != param && strParam.empty()) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                ADD_STRING_REF(strParam);
                if (!SignatureUtils::IsValidSignature(strParam.c_str())) {
                    status = ER_BUS_BAD_SIGNATURE;
                    QCC_LogError(status, ("String \"%s\" is not a legal signature", strParam.c_str()));
                    break;
                }
                arg->typeId = ALLJOYN_SIGNATURE;
                arg->v_signature.sig = strParam.c_str();
                arg->v_signature.len = strParam.length();
                if (nullptr != obj) {
                    AddObjectReference(NULL, obj, &(this->_refMap));
                }
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 'h':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_HANDLE, true);
            Platform::IBox<uint64> ^ t = dynamic_cast<Platform::IBox<uint64> ^>(obj);
            if (nullptr != t) {
                uint64 param = t->Value;
                arg->typeId = ALLJOYN_HANDLE;
                arg->v_handle.fd = (qcc::SocketFd)(void*)param;
                AddObjectReference(NULL, obj, &(this->_refMap));
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 'i':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_INT32, true);
            Platform::IBox<int32> ^ t = dynamic_cast<Platform::IBox<int32> ^>(obj);
            if (nullptr != t) {
                int32 param = t->Value;
                arg->typeId = ALLJOYN_INT32;
                arg->v_int32 = param;
                AddObjectReference(NULL, obj, &(this->_refMap));
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 'n':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_INT16, true);
            Platform::IBox<int16> ^ t = dynamic_cast<Platform::IBox<int16> ^>(obj);
            if (nullptr != t) {
                int16 param = t->Value;
                arg->typeId = ALLJOYN_INT16;
                arg->v_int16 = param;
                AddObjectReference(NULL, obj, &(this->_refMap));
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 'o':
        {
            Platform::Object ^ rawObj = args[argIndex++];
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_OBJECT_PATH, true);
            Platform::String ^ t = dynamic_cast<Platform::String ^>(obj);
            if (nullptr != obj || nullptr == rawObj) {
                Platform::String ^ param = t;
                qcc::String strParam = PlatformToMultibyteString(param);
                if (nullptr != param && strParam.empty()) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                ADD_STRING_REF(strParam);
                arg->typeId = ALLJOYN_OBJECT_PATH;
                arg->v_objPath.str = strParam.c_str();
                arg->v_objPath.len = strParam.length();
                if (nullptr != obj) {
                    AddObjectReference(NULL, obj, &(this->_refMap));
                }
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 'q':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_UINT16, true);
            Platform::IBox<uint16> ^ t = dynamic_cast<Platform::IBox<uint16> ^>(obj);
            if (nullptr != t) {
                uint16 param = t->Value;
                arg->typeId = ALLJOYN_UINT16;
                arg->v_uint16 = param;
                AddObjectReference(NULL, obj, &(this->_refMap));
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 'r':
        {
            Platform::Object ^ objVariantArray = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_ARRAY, true);
            if (nullptr == objVariantArray) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            // MsgArg []
            Platform::IBoxArray<Platform::Object ^> ^ boxArray = dynamic_cast<Platform::IBoxArray<Platform::Object ^> ^>(objVariantArray);
            if (nullptr == boxArray) {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            Platform::Array<Platform::Object ^> ^ objArray = boxArray->Value;
            if (nullptr == objArray || objArray->Length < 1) {
                status = ER_FAIL;
                break;
            }
            // Allocate an array for the msgarg pointers
            ajn::MsgArg* nativeArgs = new ajn::MsgArg[objArray->Length];
            if (NULL == nativeArgs) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            // Hold on to msgarg scratch to release later
            ADD_SCRATCH(nativeArgs);
            for (int i = 0; i < objArray->Length; i++) {
                Platform::Object ^ obj = typeCoercer->Coerce(objArray[i], ajn::ALLJOYN_VARIANT, true);
                if (nullptr == obj) {
                    // Invalid conversion
                    status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                    break;
                }
                AllJoyn::MsgArg ^ msgarg = dynamic_cast<AllJoyn::MsgArg ^>(obj);
                if (nullptr == msgarg) {
                    status = ER_FAIL;
                    break;
                }
                AddObjectReference(NULL, msgarg, &(this->_refMap));
                // Store the pointer
                ajn::MsgArg* temp = msgarg->_msgArg;
                nativeArgs[i] = *temp;
            }
            AddObjectReference(NULL, objArray, &(this->_refMap));
            arg->typeId = ALLJOYN_STRUCT;
            arg->v_struct.numMembers = objArray->Length;
            arg->v_struct.members = nativeArgs;
        }
        break;

        case 's':
        {
            Platform::Object ^ rawObj = args[argIndex++];
            Platform::Object ^ obj = typeCoercer->Coerce(rawObj, ajn::ALLJOYN_STRING, true);
            Platform::String ^ t = dynamic_cast<Platform::String ^>(obj);
            if (nullptr != obj || nullptr == rawObj) {
                Platform::String ^ param = t;
                qcc::String strParam = PlatformToMultibyteString(param);
                if (nullptr != param && strParam.empty()) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                ADD_STRING_REF(strParam);
                arg->typeId = ALLJOYN_STRING;
                arg->v_string.str = strParam.c_str();
                arg->v_string.len = strParam.length();
                if (nullptr != obj) {
                    AddObjectReference(NULL, obj, &(this->_refMap));
                }
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 't':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_UINT64, true);
            Platform::IBox<uint64> ^ t = dynamic_cast<Platform::IBox<uint64> ^>(obj);
            if (nullptr != t) {
                uint64 param = t->Value;
                arg->typeId = ALLJOYN_UINT64;
                arg->v_uint64 = param;
                AddObjectReference(NULL, obj, &(this->_refMap));
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 'u':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_UINT32, true);
            Platform::IBox<uint32> ^ t = dynamic_cast<Platform::IBox<uint32> ^>(obj);
            if (nullptr != t) {
                uint32 param = t->Value;
                arg->typeId = ALLJOYN_UINT32;
                arg->v_uint32 = param;
                AddObjectReference(NULL, obj, &(this->_refMap));
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 'v':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_VARIANT, true);
            AllJoyn::MsgArg ^ val = nullptr;
            if (nullptr != obj) {
                val = dynamic_cast<AllJoyn::MsgArg ^>(obj);
            }
            ajn::MsgArg* v = NULL;
            if (nullptr != val && NULL != val->_msgArg) {
                v = val->_msgArg;
                AddObjectReference(NULL, obj, &(this->_refMap));
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
            arg->typeId = ALLJOYN_VARIANT;
            arg->v_variant.val = v;
        }
        break;

        case 'x':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_INT64, true);
            Platform::IBox<int64> ^ t = dynamic_cast<Platform::IBox<int64> ^>(obj);
            if (nullptr != t) {
                int64 param = t->Value;
                arg->typeId = ALLJOYN_INT64;
                arg->v_int64 = param;
                AddObjectReference(NULL, obj, &(this->_refMap));
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case 'y':
        {
            Platform::Object ^ obj = typeCoercer->Coerce(args[argIndex++], ajn::ALLJOYN_BYTE, true);
            Platform::IBox<uint8> ^ t = dynamic_cast<Platform::IBox<uint8> ^>(obj);
            if (nullptr != t) {
                uint8 param = t->Value;
                arg->typeId = ALLJOYN_BYTE;
                arg->v_byte = param;
                AddObjectReference(NULL, obj, &(this->_refMap));
            } else {
                // Invalid conversion
                status = (::QStatus)((int)ER_BAD_ARG_1 + argIndex - 1);
                break;
            }
        }
        break;

        case '(':
        {
            const char* memberSig = signature;
            arg->typeId = ALLJOYN_STRUCT;
            status = SignatureUtils::ParseContainerSignature(*arg, signature);
            if (status == ER_OK) {
                // -1 to exclude the closing ')'
                size_t memSigLen = signature - memberSig - 1;
                arg->v_struct.members = new ajn::MsgArg[arg->v_struct.numMembers];
                status = VBuildArgs(memberSig, memSigLen, arg->v_struct.members, arg->v_struct.numMembers, args, argIndex, recursionLevel + 1);
                sigLen -= (memSigLen + 1);
            } else {
                QCC_LogError(status, ("Signature for STRUCT was not a complete type"));
                arg->typeId = ALLJOYN_INVALID;
                status = ER_BUS_BAD_SIGNATURE;
                break;
            }
        }
        break;

        case '{':
        {
            const char* memberSig = signature;
            arg->typeId = ALLJOYN_DICT_ENTRY;
            status = SignatureUtils::ParseContainerSignature(*arg, signature);
            if (status == ER_OK) {
                // -1 to exclude the closing '}'
                size_t memSigLen = signature - memberSig - 1;
                arg->v_dictEntry.key = new ajn::MsgArg;
                arg->v_dictEntry.val = new ajn::MsgArg;
                status = VBuildArgs(memberSig, memSigLen, arg->v_dictEntry.key, 1, args, argIndex, recursionLevel + 1);
                if (status != ER_OK) {
                    break;
                }
                status = VBuildArgs(memberSig, memSigLen, arg->v_dictEntry.val, 1, args, argIndex, recursionLevel + 1);
                if (status != ER_OK) {
                    break;
                }
                sigLen -= (memSigLen + 1);
            } else {
                QCC_LogError(status, ("Signature for DICT_ENTRY was not a complete type"));
                arg->typeId = ALLJOYN_INVALID;
                status = ER_BUS_BAD_SIGNATURE;
                break;
            }
        }
        break;

        default:
            QCC_LogError(ER_BUS_BAD_SIGNATURE, ("Invalid char '\\%d' in signature", *(signature - 1)));
            arg->typeId = ALLJOYN_INVALID;
            status = ER_BUS_BAD_SIGNATURE;
            break;
        }

        if (status != ER_OK) {
            arg->Clear();
            break;
        }

        arg++;
    }

    if (ER_OK == status && 0 == recursionLevel) {
        if (*signature != '\0') {
            arg->Clear();
            status = ER_BUS_BAD_SIGNATURE;
        } else if (argIndex != args->Length) {
            arg->Clear();
            status = ER_BAD_ARG_COUNT;
        }
    }

    return status;
}

void _MsgArg::SetObject(AllJoyn::MsgArg ^ arg, bool isKey)
{
    ::QStatus status = ER_OK;

    switch (arg->_msgArg->typeId) {
    case ALLJOYN_BOOLEAN:
    {
        Platform::Object ^ obj = PropertyValue::CreateBoolean(arg->_msgArg->v_bool);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_BOOLEAN, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_BOOLEAN, false);
        }
    }
    break;

    case ALLJOYN_DOUBLE:
    {
        Platform::Object ^ obj = PropertyValue::CreateDouble(arg->_msgArg->v_double);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_DOUBLE, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_DOUBLE, false);
        }
    }
    break;

    case ALLJOYN_DICT_ENTRY:
    {
        AllJoyn::MsgArg ^ newKey = ref new AllJoyn::MsgArg(arg->_msgArg->v_dictEntry.key);
        AllJoyn::MsgArg ^ newValue = ref new AllJoyn::MsgArg(arg->_msgArg->v_dictEntry.val);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(newKey->Value, arg->_msgArg->v_dictEntry.key->typeId, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(newValue->Value, arg->_msgArg->v_dictEntry.val->typeId, false);
        }
    }
    break;

    case ALLJOYN_INT32:
    {
        Platform::Object ^ obj = PropertyValue::CreateInt32(arg->_msgArg->v_int32);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT32, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT32, false);
        }
    }
    break;

    case ALLJOYN_STRUCT:
    {
        ajn::MsgArg* elements = arg->_msgArg->v_struct.members;
        size_t elementCount = arg->_msgArg->v_struct.numMembers;
        Platform::Array<Platform::Object ^> ^ arr = ref new Platform::Array<Platform::Object ^>(elementCount);
        for (int i = 0; i < elementCount; i++) {
            AllJoyn::MsgArg ^ newArg = ref new AllJoyn::MsgArg(&(elements[i]));
            AddObjectReference(NULL, newArg, &(this->_refMap));
            SetObject(newArg, isKey);
            if (isKey) {
                arr[i] = newArg->_msgArg->_eventsAndProperties->Key;
            } else {
                arr[i] = newArg->_msgArg->_eventsAndProperties->Value;
            }
        }
        Platform::Object ^ obj = PropertyValue::CreateInspectableArray(arr);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_ARRAY, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_ARRAY, false);
        }
    }
    break;

    case ALLJOYN_STRING:
    {
        qcc::String val = arg->_msgArg->v_string.str;
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(MultibyteToPlatformString(val.c_str()), ajn::ALLJOYN_STRING, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(MultibyteToPlatformString(val.c_str()), ajn::ALLJOYN_STRING, false);
        }
    }
    break;

    case ALLJOYN_VARIANT:
    {
        AllJoyn::MsgArg ^ newArg = ref new AllJoyn::MsgArg(arg->_msgArg->v_variant.val);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(newArg, ajn::ALLJOYN_VARIANT, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(newArg, ajn::ALLJOYN_VARIANT, false);
        }
    }
    break;

    case ALLJOYN_INT64:
    {
        Platform::Object ^ obj = PropertyValue::CreateInt64(arg->_msgArg->v_int64);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT64, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT64, false);
        }
    }
    break;

    case ALLJOYN_BYTE:
    {
        Platform::Object ^ obj = PropertyValue::CreateUInt8(arg->_msgArg->v_byte);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_BYTE, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_BYTE, false);
        }
    }
    break;

    case ALLJOYN_UINT32:
    {
        Platform::Object ^ obj = PropertyValue::CreateUInt32(arg->_msgArg->v_uint32);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT32, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT32, false);
        }
    }
    break;

    case ALLJOYN_UINT64:
    {
        Platform::Object ^ obj = PropertyValue::CreateUInt64(arg->_msgArg->v_uint64);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT64, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT64, false);
        }
    }
    break;

    case ALLJOYN_OBJECT_PATH:
    {
        qcc::String val = arg->_msgArg->v_objPath.str;
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(MultibyteToPlatformString(val.c_str()), ajn::ALLJOYN_OBJECT_PATH, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(MultibyteToPlatformString(val.c_str()), ajn::ALLJOYN_OBJECT_PATH, false);
        }
    }
    break;

    case ALLJOYN_SIGNATURE:
    {
        qcc::String val = arg->_msgArg->v_signature.sig;
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(MultibyteToPlatformString(val.c_str()), ajn::ALLJOYN_SIGNATURE, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(MultibyteToPlatformString(val.c_str()), ajn::ALLJOYN_SIGNATURE, false);
        }
    }
    break;

    case ALLJOYN_HANDLE:
    {
        Platform::Object ^ obj = PropertyValue::CreateUInt64(arg->_msgArg->v_handle.fd);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_HANDLE, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_HANDLE, false);
        }
    }
    break;

    case ALLJOYN_UINT16:
    {
        Platform::Object ^ obj = PropertyValue::CreateUInt16(arg->_msgArg->v_uint16);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT16, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT16, false);
        }
    }
    break;

    case ALLJOYN_INT16:
    {
        Platform::Object ^ obj = PropertyValue::CreateInt16(arg->_msgArg->v_int16);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT16, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT16, false);
        }
    }
    break;

    case ALLJOYN_ARRAY:
    {
        ajn::MsgArg* elements = (ajn::MsgArg*)arg->_msgArg->v_array.GetElements();
        if (NULL != elements) {
            size_t elementCount = arg->_msgArg->v_array.GetNumElements();
            Platform::Array<Platform::Object ^> ^ arr = ref new Platform::Array<Platform::Object ^>(elementCount);
            for (int i = 0; i < elementCount; i++) {
                AllJoyn::MsgArg ^ newArg = ref new AllJoyn::MsgArg(&(elements[i]));
                AddObjectReference(NULL, newArg, &(this->_refMap));
                arr[i] = newArg;
            }
            Platform::Object ^ obj = PropertyValue::CreateInspectableArray(arr);
            if (isKey) {
                arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_ARRAY, false);
            } else {
                arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_ARRAY, false);
            }
        }
    }
    break;

    case ALLJOYN_BOOLEAN_ARRAY:
    {
        Platform::ArrayReference<Platform::Boolean> arrRef((bool*)arg->_msgArg->v_scalarArray.v_bool,
                                                           arg->_msgArg->v_scalarArray.numElements);
        Platform::Object ^ obj = PropertyValue::CreateBooleanArray(arrRef);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_BOOLEAN_ARRAY, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_BOOLEAN_ARRAY, false);
        }
    }
    break;

    case ALLJOYN_DOUBLE_ARRAY:
    {
        Platform::ArrayReference<float64> arrRef((float64*)arg->_msgArg->v_scalarArray.v_double,
                                                 arg->_msgArg->v_scalarArray.numElements);
        Platform::Object ^ obj = PropertyValue::CreateDoubleArray(arrRef);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_DOUBLE_ARRAY, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_DOUBLE_ARRAY, false);
        }
    }
    break;

    case ALLJOYN_INT32_ARRAY:
    {
        Platform::ArrayReference<int32> arrRef((int32*)arg->_msgArg->v_scalarArray.v_int32,
                                               arg->_msgArg->v_scalarArray.numElements);
        Platform::Object ^ obj = PropertyValue::CreateInt32Array(arrRef);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT32_ARRAY, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT32_ARRAY, false);
        }
    }
    break;

    case ALLJOYN_INT16_ARRAY:
    {
        Platform::ArrayReference<int16> arrRef((int16*)arg->_msgArg->v_scalarArray.v_int16,
                                               arg->_msgArg->v_scalarArray.numElements);
        Platform::Object ^ obj = PropertyValue::CreateInt16Array(arrRef);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT16_ARRAY, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT16_ARRAY, false);
        }
    }
    break;

    case ALLJOYN_UINT16_ARRAY:
    {
        Platform::ArrayReference<uint16> arrRef((uint16*)arg->_msgArg->v_scalarArray.v_uint16,
                                                arg->_msgArg->v_scalarArray.numElements);
        Platform::Object ^ obj = PropertyValue::CreateUInt16Array(arrRef);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT16_ARRAY, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT16_ARRAY, false);
        }
    }
    break;

    case ALLJOYN_UINT64_ARRAY:
    {
        Platform::ArrayReference<uint64> arrRef((uint64*)arg->_msgArg->v_scalarArray.v_uint64,
                                                arg->_msgArg->v_scalarArray.numElements);
        Platform::Object ^ obj = PropertyValue::CreateUInt64Array(arrRef);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT64_ARRAY, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT64_ARRAY, false);
        }
    }
    break;

    case ALLJOYN_UINT32_ARRAY:
    {
        Platform::ArrayReference<uint32> arrRef((uint32*)arg->_msgArg->v_scalarArray.v_uint32,
                                                arg->_msgArg->v_scalarArray.numElements);
        Platform::Object ^ obj = PropertyValue::CreateUInt32Array(arrRef);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT32_ARRAY, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_UINT32_ARRAY, false);
        }
    }
    break;

    case ALLJOYN_INT64_ARRAY:
    {
        Platform::ArrayReference<int64> arrRef((int64*)arg->_msgArg->v_scalarArray.v_int64,
                                               arg->_msgArg->v_scalarArray.numElements);
        Platform::Object ^ obj = PropertyValue::CreateInt64Array(arrRef);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT64_ARRAY, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_INT64_ARRAY, false);
        }
    }
    break;

    case ALLJOYN_BYTE_ARRAY:
    {
        Platform::ArrayReference<uint8> arrRef((uint8*)arg->_msgArg->v_scalarArray.v_byte,
                                               arg->_msgArg->v_scalarArray.numElements);
        Platform::Object ^ obj = PropertyValue::CreateUInt8Array(arrRef);
        if (isKey) {
            arg->_msgArg->_eventsAndProperties->Key = typeCoercer->Coerce(obj, ajn::ALLJOYN_BYTE_ARRAY, false);
        } else {
            arg->_msgArg->_eventsAndProperties->Value = typeCoercer->Coerce(obj, ajn::ALLJOYN_BYTE_ARRAY, false);
        }
    }
    break;

    default:
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

__MsgArg::__MsgArg()
{
    Value = nullptr;
    Key = nullptr;
}

__MsgArg::~__MsgArg()
{
    Value = nullptr;
    Key = nullptr;
}

}

