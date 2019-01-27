using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.IO;

namespace PolygonCuttingEarInterface
{
	public class frmHelp_SimplePolygon : PolygonCuttingEarInterface.frmHelpWindow
	{
		private System.ComponentModel.IContainer components = null;

		public frmHelp_SimplePolygon()
		{
			// This call is required by the Windows Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitializeComponent call
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			// 
			// frmHelp_SimplePolygon
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(474, 255);
			this.Name = "frmHelp_SimplePolygon";
			this.Text = "Help: Simple Polygon";
			this.TopMost = false;
			this.Load += new System.EventHandler(this.frmHelp_SimplePolygon_Load);

		}
		#endregion

		private void frmHelp_SimplePolygon_Load(object sender, System.EventArgs e)
		{
			
		}

		protected void rtbDescription_LinkClicked(object sender, System.Windows.Forms.LinkClickedEventArgs e)
		{
			
		}
	}
}

