//-----------------------------------------------------------------------
// <copyright file="MsgArgTests.cs" company="Qualcomm Innovation Center, Inc.">
//     Copyright 2012, Qualcomm Innovation Center, Inc.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
// </copyright>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using AllJoyn;

namespace AllJoynUnitTests
{
    [TestClass]
    public class MsgArgTests
    {
        struct SimpleStruct1
        {
            public Int32 i;
            public string s;
            public bool b;
            public byte y;
            public SimpleStruct1(Int32 i1, string s1, bool b1, byte y1) { i = i1; s = s1; b = b1; y = y1; }
            public bool Equals(SimpleStruct1 other) { return (i == other.i) && (s == other.s) && (b == other.b) && (y == other.y); }
        }
        struct SimpleStruct2
        {
            public double d;
            public Int64 x;
            public UInt64 t;
            public Int16 n;
            public SimpleStruct2(double i1, Int64 s1, UInt64 b1, Int16 y1) { d = i1; x = s1; t = b1; n = y1; }
            public bool Equals(SimpleStruct2 other) { return (d == other.d) && (x == other.x) && (t == other.t) && (n == other.n); }
        }
        struct SimpleStruct3
        {
            public UInt16 q;
            public bool b;
            public UInt32 u;
            public double d;
            public SimpleStruct3(UInt16 i1, bool s1, UInt32 b1, double y1) { q = i1; b = s1; u = b1; d = y1; }
            public bool Equals(SimpleStruct3 other) { return (q == other.q) && (b == other.b) && (u == other.u) && (d == other.d); }
        }
        struct TestStruct
        {
            public Int32 i;
            public string s;
        }

        [TestMethod]
        public void PackAndUnpackingSimpleMsgArgs()
        {
            bool b1 = true;
            bool b2 = false;
            byte y1 = 0;
            byte y2 = 255;
            Int16 n1 = Int16.MaxValue;
            Int16 n2 = Int16.MinValue;
            UInt16 q1 = UInt16.MaxValue;
            UInt16 q2 = UInt16.MinValue;
            Int32 i1 = Int32.MaxValue;
            Int32 i2 = Int32.MinValue;
            UInt32 u1 = UInt32.MaxValue;
            UInt32 u2 = UInt32.MinValue;
            Int64 x1 = Int64.MaxValue;
            Int64 x2 = Int64.MinValue;
            UInt64 t1 = UInt64.MaxValue;
            UInt64 t2 = UInt64.MinValue;
            double d1 = double.MaxValue;
            double d2 = double.MinValue;
            string s1 = "string one";
            string s2 = "string two";
            bool[] bArray = new bool[] { true, true, false, true, false };
            byte[] yArray = new byte[] { 1, 7, 13, 27, 43, 111, 137, 231 };
            Int16[] nArray = new Int16[] { -231, -137, -111, -43, -27, -13, -7, -1, 1, 7, 13, 27, 43, 111, 137, 231 };
            UInt16[] qArray = new UInt16[] { 1, 7, 13, 27, 43, 111, 137, 231 };
            Int32[] iArray = new Int32[] { -231, -137, -111, -43, -27, -13, -7, -1, 1, 7, 13, 27, 43, 111, 137, 231 };
            UInt32[] uArray = new UInt32[] { 1, 7, 13, 27, 43, 111, 137, 231 };
            Int64[] xArray = new Int64[] { -231, -137, -111, -43, -27, -13, -7, -1, 1, 7, 13, 27, 43, 111, 137, 231 };
            UInt64[] tArray = new UInt64[] { 1, 7, 13, 27, 43, 111, 137, 231 };
            double[] dArray = new double[] { -1235.3476, -5.456366, 0.0, 4.56566, 12344.2345 };
            string[] sArray = new string[] { "this", "is", "a", "string", "array" };

            MsgArg bArg1 = new MsgArg("b", new object[] { b1 });
            MsgArg bArg2 = new MsgArg("b", new object[] { b2 });
            Assert.AreEqual(b1, (bool)bArg1.Value);
            Assert.AreEqual(b2, (bool)bArg2.Value);

            MsgArg yArg1 = new MsgArg("y", new object[] { y1 });
            MsgArg yArg2 = new MsgArg("y", new object[] { y2 });
            Assert.AreEqual(y1, (byte)yArg1.Value);
            Assert.AreEqual(y2, (byte)yArg2.Value);

            MsgArg nArg1 = new MsgArg("n", new object[] { n1 });
            MsgArg nArg2 = new MsgArg("n", new object[] { n2 });
            Assert.AreEqual(n1, (Int16)nArg1.Value);
            Assert.AreEqual(n2, (Int16)nArg2.Value);

            MsgArg qArg1 = new MsgArg("q", new object[] { q1 });
            MsgArg qArg2 = new MsgArg("q", new object[] { q2 });
            Assert.AreEqual(q1, (UInt16)qArg1.Value);
            Assert.AreEqual(q2, (UInt16)qArg2.Value);

            MsgArg iArg1 = new MsgArg("i", new object[] { i1 });
            MsgArg iArg2 = new MsgArg("i", new object[] { i2 });
            Assert.AreEqual(i1, (Int32)iArg1.Value);
            Assert.AreEqual(i2, (Int32)iArg2.Value);

            MsgArg uArg1 = new MsgArg("u", new object[] { u1 });
            MsgArg uArg2 = new MsgArg("u", new object[] { u2 });
            Assert.AreEqual(u1, (UInt32)uArg1.Value);
            Assert.AreEqual(u2, (UInt32)uArg2.Value);

            MsgArg xArg1 = new MsgArg("x", new object[] { x1 });
            MsgArg xArg2 = new MsgArg("x", new object[] { x2 });
            Assert.AreEqual(x1, (Int64)xArg1.Value);
            Assert.AreEqual(x2, (Int64)xArg2.Value);

            MsgArg tArg1 = new MsgArg("t", new object[] { t1 });
            MsgArg tArg2 = new MsgArg("t", new object[] { t2 });
            Assert.AreEqual(t1, (UInt64)tArg1.Value);
            Assert.AreEqual(t2, (UInt64)tArg2.Value);

            MsgArg dArg1 = new MsgArg("d", new object[] { d1 });
            MsgArg dArg2 = new MsgArg("d", new object[] { d2 });
            Assert.AreEqual(d1, (double)dArg1.Value);
            Assert.AreEqual(d2, (double)dArg2.Value);

            MsgArg sArg1 = new MsgArg("s", new object[] { s1 });
            MsgArg sArg2 = new MsgArg("s", new object[] { s2 });
            Assert.AreEqual(s1, (string)sArg1.Value);
            Assert.AreEqual(s2, (string)sArg2.Value);

            MsgArg abArg = new MsgArg("ab", new object[] { bArray });
            CompareArrays<bool>(bArray, (bool[])abArg.Value);

            MsgArg ayArg = new MsgArg("ay", new object[] { yArray });
            CompareArrays<byte>(yArray, (byte[])ayArg.Value);

            MsgArg anArg = new MsgArg("an", new object[] { nArray });
            CompareArrays<Int16>(nArray, (Int16[])anArg.Value);

            MsgArg aqArg = new MsgArg("aq", new object[] { qArray });
            CompareArrays<UInt16>(qArray, (UInt16[])aqArg.Value);

            MsgArg aiArg = new MsgArg("ai", new object[] { iArray });
            CompareArrays<Int32>(iArray, (Int32[])aiArg.Value);

            MsgArg auArg = new MsgArg("au", new object[] { uArray });
            CompareArrays<UInt32>(uArray, (UInt32[])auArg.Value);

            MsgArg axArg = new MsgArg("ax", new object[] { xArray });
            CompareArrays<Int64>(xArray, (Int64[])axArg.Value);

            MsgArg atArg = new MsgArg("at", new object[] { tArray });
            CompareArrays<UInt64>(tArray, (UInt64[])atArg.Value);

            MsgArg adArg = new MsgArg("ad", new object[] { dArray });
            CompareArrays<double>(dArray, (double[])adArg.Value);

            MsgArg asArg = new MsgArg("as", new object[] { sArray });
            // BUGBUG: Throws exception when casting asArg.Value to string[]
            //CompareArrays<string>(sArray, (string[])asArg.Value);

            MsgArg dictArg1 = new MsgArg("{yb}", new object[] { y1, b1 });
            Assert.AreEqual(y1, (byte)dictArg1.Key);
            Assert.AreEqual(b1, (bool)dictArg1.Value);
            MsgArg dictArg2 = new MsgArg("{by}", new object[] { b2, y2 });
            Assert.AreEqual(b2, (bool)dictArg2.Key);
            Assert.AreEqual(y2, (byte)dictArg2.Value);

            MsgArg dictArg3 = new MsgArg("{bn}", new object[] { b1, n1 });
            Assert.AreEqual(b1, (bool)dictArg3.Key);
            Assert.AreEqual(n1, (Int16)dictArg3.Value);
            MsgArg dictArg4 = new MsgArg("{nb}", new object[] { n2, b2 });
            Assert.AreEqual(n2, (Int16)dictArg4.Key);
            Assert.AreEqual(b2, (bool)dictArg4.Value);

            MsgArg dictArg5 = new MsgArg("{nq}", new object[] { n1, q1 });
            Assert.AreEqual(n1, (Int16)dictArg5.Key);
            Assert.AreEqual(q1, (UInt16)dictArg5.Value);
            MsgArg dictArg6 = new MsgArg("{qn}", new object[] { q2, n2 });
            Assert.AreEqual(q2, (UInt16)dictArg6.Key);
            Assert.AreEqual(n2, (Int16)dictArg6.Value);

            MsgArg dictArg7 = new MsgArg("{qi}", new object[] { q1, i1 });
            Assert.AreEqual(q1, (UInt16)dictArg7.Key);
            Assert.AreEqual(i1, (Int32)dictArg7.Value);
            MsgArg dictArg8 = new MsgArg("{iq}", new object[] { i2, q2 });
            Assert.AreEqual(i2, (Int32)dictArg8.Key);
            Assert.AreEqual(q2, (UInt16)dictArg8.Value);

            MsgArg dictArg9 = new MsgArg("{iu}", new object[] { i1, u1 });
            Assert.AreEqual(i1, (Int32)dictArg9.Key);
            Assert.AreEqual(u1, (UInt32)dictArg9.Value);
            MsgArg dictArg10 = new MsgArg("{ui}", new object[] { u2, i2 });
            Assert.AreEqual(u2, (UInt32)dictArg10.Key);
            Assert.AreEqual(i2, (Int32)dictArg10.Value);

            MsgArg dictArg11 = new MsgArg("{ux}", new object[] { u1, x1 });
            Assert.AreEqual(u1, (UInt32)dictArg11.Key);
            Assert.AreEqual(x1, (Int64)dictArg11.Value);
            MsgArg dictArg12 = new MsgArg("{xu}", new object[] { x2, u2 });
            Assert.AreEqual(x2, (Int64)dictArg12.Key);
            Assert.AreEqual(u2, (UInt32)dictArg12.Value);

            MsgArg dictArg13 = new MsgArg("{xt}", new object[] { x1, t1 });
            Assert.AreEqual(x1, (Int64)dictArg13.Key);
            Assert.AreEqual(t1, (UInt64)dictArg13.Value);
            MsgArg dictArg14 = new MsgArg("{tx}", new object[] { t2, x2 });
            Assert.AreEqual(t2, (UInt64)dictArg14.Key);
            Assert.AreEqual(x2, (Int64)dictArg14.Value);

            MsgArg dictArg15 = new MsgArg("{td}", new object[] { t1, d1 });
            Assert.AreEqual(t1, (UInt64)dictArg15.Key);
            Assert.AreEqual(d1, (double)dictArg15.Value);
            MsgArg dictArg16 = new MsgArg("{dt}", new object[] { d2, t2 });
            Assert.AreEqual(d2, (double)dictArg16.Key);
            Assert.AreEqual(t2, (UInt64)dictArg16.Value);

            MsgArg dictArg17 = new MsgArg("{ds}", new object[] { d1, s1 });
            Assert.AreEqual(d1, (double)dictArg17.Key);
            Assert.AreEqual(s1, (string)dictArg17.Value);
            MsgArg dictArg18 = new MsgArg("{sd}", new object[] { s2, d2 });
            Assert.AreEqual(s2, (string)dictArg18.Key);
            Assert.AreEqual(d2, (double)dictArg18.Value);

            // Exception is being thrown which says :
            // System.Exception: A concurrent or interleaved operation changed the state of the object, invalidating this operation.
            // This gets thrown when wrapping struct in MsgArg below
            TestStruct ts = new TestStruct();
            ts.i = 1;
            ts.s = "string";
            MsgArg sArg = new MsgArg("(is)", new object[] { ts });
            Assert.AreEqual(ts.i, ((TestStruct)sArg.Value).i);
            Assert.AreEqual(ts.s, ((TestStruct)sArg.Value).s);
            Assert.AreEqual(ts, (TestStruct)sArg.Value);

            SimpleStruct1 struct1 = new SimpleStruct1(6566566, "simplestruct", true, 167);
            MsgArg structArg1 = new MsgArg("(isby)", new object[] { struct1 });
            Assert.AreEqual(struct1, (SimpleStruct1)structArg1.Value);

            SimpleStruct2 struct2 = new SimpleStruct2(2345.3344, -789342, 7832487934, 134);
            MsgArg structArg2 = new MsgArg("(dxtn)", new object[] { struct2 });
            Assert.AreEqual(struct2, (SimpleStruct2)structArg2.Value);

            SimpleStruct3 struct3 = new SimpleStruct3(255, false, 838954, 66.36566);
            MsgArg structArg3 = new MsgArg("(qbud)", new object[] { struct3 });
            Assert.AreEqual(struct3, (SimpleStruct3)structArg3.Value);
        }

        public void CompareArrays<T>(T[] a1, T[] a2)
        {
            Assert.AreEqual(a1.Length, a2.Length);
            for (int i = 0; i < a1.Length; i++)
            {
                Assert.AreEqual(a1[i], a2[i]);
            }
        }

        public Random rand = new Random();
        struct Struct1
        {
            public UInt32 u;
            public Struct2 st1;
            public string s;
            public double d;
            public Struct2 st2;
            public Int32 i;
            public UInt16 q;
            public Struct2 st3;
            public bool b;
            public Struct1(UInt32 u1, Struct2 stk1, string s1, double d1, Struct2 stk2, Int32 i1, UInt16 q1, Struct2 stk3, bool b1)
            {
                u = u1; st1 = stk1; s = s1; d = d1; st2 = stk2; i = i1; q = q1; st3 = stk3; b = b1;
            }
            public bool Equals(Struct1 other)
            {
                return (u == other.u) && (st1.Equals(other.st1)) && (s == other.s) && (d == other.d) && (st2.Equals(other.st2)) && 
                    (i == other.i) && (q == other.q) && (st3.Equals(other.st3)) && (b == other.b);
            }
        }
        struct Struct2
        {
            public byte y;
            public Int64 x;
            public double d;
            public Struct2(byte y1, Int64 x1, double d1) { y = y1; x = x1; d = d1; }
            public bool Equals(Struct2 other) { return (y == other.y) && (x == other.x) && (d == other.d); }
        }

        [TestMethod]
        public void PackAndUnpackingComplexMsgArgs()
        {
            //////////////////////////////////////////////////////////////////////////
            Int32[,] aai = new Int32[5, 10];
            lock (aai)
            {
                for (int i = 0; i < 5; i++)
                {
                    for (int j = 0; j < 10; j++)
                    {
                        aai[i, j] = rand.Next(Int32.MinValue, Int32.MaxValue);
                    }
                }
            }
            lock (aai)
            {
                // Exception is being thrown which says :
                // System.Exception: A concurrent or interleaved operation changed the state of the object, invalidating this operation.
                // This gets thrown when wrapping multidimensional array in MsgArg below
                MsgArg aaiArg = new MsgArg("aai", new object[] { aai });
                Int32[,] aaiUnpacked = (Int32[,])aaiArg.Value;
                Compare2DimArrays<Int32>(aai, aaiUnpacked);
            }
            //////////////////////////////////////////////////////////////////////////

            string sig1 = "(u(yxd)sd(yxd)iq(yxd)b)";
            Struct2 s1 = new Struct2(212, 35683450634546, 39.435723);
            Struct2 s2 = new Struct2(111, -2346656767823, -34.83457);
            Struct2 s3 = new Struct2(7, 3245445, 732.234);
            Struct1 s4 = new Struct1(8743, s1, "testingthestructs", 2456.666, s2, -36566, 34, s3, false);
            MsgArg sig1Arg = new MsgArg(sig1, new object[] { s4 });
            Assert.AreEqual(s4, (Struct1)sig1Arg.Value);


            string sig2 = "{aai(yxd)}";
            MsgArg dictArg = new MsgArg(sig2, new object[] { aai, s1 });
            Compare2DimArrays<Int32>(aai, (Int32[,])dictArg.Key);
            Assert.AreEqual(s1, (Struct2)dictArg.Value);



        }

        public void Compare2DimArrays<T>(T[,] one, T[,] two)
        {
            Assert.AreEqual(one.GetLength(0), two.GetLength(0));
            Assert.AreEqual(one.GetLength(1), two.GetLength(1));
            for (int i = 0; i < one.GetLength(0); i++)
            {
                for (int j = 0; j < one.GetLength(1); j++)
                {
                    Assert.AreEqual(one[i, j], two[i, j]);
                }
            }
        }
    }
}
