using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.IO;

namespace PolygonCuttingEarInterface
{
	public class frmHelp_About : PolygonCuttingEarInterface.frmHelpWindow
	{
		private System.ComponentModel.IContainer components = null;

		public frmHelp_About()
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
			// frmHelp_About
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(474, 255);
			this.Name = "frmHelp_About";
			this.Text = "Help: About This Program";
			this.TopMost = false;
			this.Load += new System.EventHandler(this.frmHelp_About_Load);

		}
		#endregion

		private void frmHelp_About_Load(object sender, System.EventArgs e)
		{
			
		}
	}
}

