using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.IO;

namespace PolygonCuttingEarInterface
{
	/// <summary>
	/// Summary description for Help_SimplePolygon.
	/// </summary>
	public class frmHelpWindow : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Button btnClose;
		private System.Windows.Forms.RichTextBox rtbDescription;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public frmHelpWindow()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.rtbDescription = new System.Windows.Forms.RichTextBox();
			this.btnClose = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// rtbDescription
			// 
			this.rtbDescription.Dock = System.Windows.Forms.DockStyle.Top;
			this.rtbDescription.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.rtbDescription.Location = new System.Drawing.Point(0, 0);
			this.rtbDescription.Name = "rtbDescription";
			this.rtbDescription.Size = new System.Drawing.Size(418, 208);
			this.rtbDescription.TabIndex = 0;
			this.rtbDescription.Text = "";
			this.rtbDescription.LinkClicked += new System.Windows.Forms.LinkClickedEventHandler(this.rtbDescription_LinkClicked);
			// 
			// btnClose
			// 
			this.btnClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.btnClose.Location = new System.Drawing.Point(128, 216);
			this.btnClose.Name = "btnClose";
			this.btnClose.Size = new System.Drawing.Size(136, 32);
			this.btnClose.TabIndex = 5;
			this.btnClose.Text = "Clo&se";
			this.btnClose.Click += new System.EventHandler(this.btnClose_Click);
			// 
			// frmHelpWindow
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(418, 263);
			this.ControlBox = false;
			this.Controls.Add(this.btnClose);
			this.Controls.Add(this.rtbDescription);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.Name = "frmHelpWindow";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
			this.Text = "Help: ";
			this.TopMost = true;
			this.Load += new System.EventHandler(this.frmHelpWindow_Load);
			this.ResumeLayout(false);

		}
		#endregion

		private void btnClose_Click(object sender, System.EventArgs e)
		{
			this.Close();
		}

		private void frmHelpWindow_Load(object sender, System.EventArgs e)
		{
			if (sender is frmHelp_SimplePolygon) 
			{
				if (File.Exists("Help_SimplePolygon.rtf"))
					rtbDescription.LoadFile("Help_SimplePolygon.rtf");
				else
					rtbDescription.Text="Please visit http://www.simplepolygon.com to "
						+ "learn more about Simple Polygon.";
			}
			else if (sender is frmHelp_About)
			{
				if (File.Exists("Help_About.rtf"))
					this.rtbDescription.LoadFile("Help_About.rtf");
				else
					rtbDescription.Text="This Program is developed by Frank Shen\n" +
						"Date: August 2004.";
			}
		}

		private void rtbDescription_LinkClicked(object sender, System.Windows.Forms.LinkClickedEventArgs e)
		{
			System.Diagnostics.Process.Start(
				"IExplore", e.LinkText);
		}
	}
}
