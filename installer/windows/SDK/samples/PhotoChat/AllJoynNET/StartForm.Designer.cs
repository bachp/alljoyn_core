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

namespace AllJoynNET {
partial class StartForm {
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing)
    {
        if (disposing && (components != null)) {
            components.Dispose();
        }
        base.Dispose(disposing);
    }

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
        System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(StartForm));
        this.btnStart = new System.Windows.Forms.Button();
        this.SuspendLayout();
        //
        // btnStart
        //
        this.btnStart.Location = new System.Drawing.Point(104, 198);
        this.btnStart.Name = "btnStart";
        this.btnStart.Size = new System.Drawing.Size(75, 23);
        this.btnStart.TabIndex = 0;
        this.btnStart.Text = "Start";
        this.btnStart.UseVisualStyleBackColor = true;
        this.btnStart.Click += new System.EventHandler(this.btnStart_Click);
        //
        // StartForm
        //
        this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
        this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
        this.BackColor = System.Drawing.SystemColors.ButtonFace;
        this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
        this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
        this.ClientSize = new System.Drawing.Size(282, 255);
        this.Controls.Add(this.btnStart);
        this.Name = "StartForm";
        this.Text = "StartForm";
        this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.Button btnStart;
}
}
