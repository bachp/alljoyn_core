// ****************************************************************************
// Copyright 2011, Qualcomm Innovation Center, Inc.
//
//     Licensed under the Apache License, Version 2.0 (the "License");
//     you may not use this file except in compliance with the License.
//     You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//     Unless required by applicable law or agreed to in writing, software
//     distributed under the License is distributed on an "AS IS" BASIS,
//     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//     See the License for the specific language governing permissions and
//     limitations under the License.
// ******************************************************************************

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace PhotoChat {
public partial class AllJoynConnect : Form {
    public AllJoynConnect()
    {
        _owner = null;
        InitializeComponent();
        this.Hide();
        //            _alljoyn = new AllJoynChatComponant();
    }

    public AllJoynConnect(Form owner)
    {
        _owner = owner;
        InitializeComponent();
        this.Hide();
        //            _alljoyn = new AllJoynChatComponant();
    }

    //        public AllJoynChatComponant Alljoyn { get { return _alljoyn; } }

    public bool IsConnected { get { return _connected; } set { _connected = value; } }
    public string SessionName { get { return txtSession.Text; } }
    public string MyHandle { get { return txtHandle.Text; } }
    public bool IsNameOwner { get { return rbAdvertise.Checked; } }

    //        private AllJoynChatComponant _alljoyn = null;
    private Form _owner = null;
    private bool _connected = false;

    protected override void OnShown(EventArgs e)
    {
        if (IsConnected) {
            btnOk.Text = "Disconnect";
            txtSession.Enabled = false;
            txtHandle.Enabled = false;
            rbAdvertise.Enabled = false;
            rbJoin.Enabled = false;
        } else   {
            btnOk.Text = "Connect";
            txtSession.Enabled = true;
            txtHandle.Enabled = true;
            rbAdvertise.Enabled = true;
            rbJoin.Enabled = true;
        }
        base.OnShown(e);
    }
}
}

