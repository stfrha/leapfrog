using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Drawing.Drawing2D;

using GeometryUtility;
using PolygonCuttingEar;

namespace PolygonCuttingEarInterface
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class frmCuttingEars : System.Windows.Forms.Form
	{
		private System.Windows.Forms.ContextMenu cmnuCuttingEar;
		private System.Windows.Forms.GroupBox gbPolygon;
		private System.Windows.Forms.RadioButton rdoTestData;
		private System.Windows.Forms.RadioButton rdoSelect;
		private System.Windows.Forms.ToolBar tbCuttingEar;
		private System.Windows.Forms.MainMenu mnuCuttingEar;
		private System.Windows.Forms.Button btnClose;
		private System.Windows.Forms.Button btnCut;
		private System.Windows.Forms.Button btnDraw;
		private System.Windows.Forms.Button btnClear;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.Panel pnlDraw;
		private System.Windows.Forms.MenuItem mnuPolygon;
		private System.Windows.Forms.MenuItem mnuTestData;
		private System.Windows.Forms.MenuItem mnuMouseClick;
		private System.Windows.Forms.MenuItem mnuDraw;
		private System.Windows.Forms.MenuItem mnuDrawPolygon;
		private System.Windows.Forms.MenuItem mnuHelp;
		private System.Windows.Forms.MenuItem mnuEar;
		private System.Windows.Forms.MenuItem mnuRedraw;

		private Point[] m_aPolygonPts;
		private ArrayList m_alSelectedPts=new ArrayList(); 

		private Pen m_PolygonPen=new Pen(Color.Blue, 2);
		private System.Windows.Forms.ToolTip ttpPolygon;
		private System.Windows.Forms.MenuItem cmnuDrawPolygon;
		private System.Windows.Forms.MenuItem cmnCut;
		private System.Windows.Forms.MenuItem cmnuClean;
		private System.Windows.Forms.MenuItem cmnuClose;
		private System.Windows.Forms.MenuItem mnuSimplePolygon;
		private System.Windows.Forms.MenuItem mnuAbout;
		private System.Windows.Forms.ToolBarButton tbbDraw;
		private System.Windows.Forms.ToolBarButton tbbCut;
		private System.Windows.Forms.ToolBarButton tbbClean;
		private System.Windows.Forms.ToolBarButton tbbClose;
		private System.Windows.Forms.ToolBarButton tbbSeparator;
		private System.Windows.Forms.ImageList ilToolbar;
		private System.Windows.Forms.Label lblDesc;
		private System.Windows.Forms.StatusBar sbDesc;
		private System.Windows.Forms.StatusBarPanel sbpDes;
		private System.Windows.Forms.StatusBarPanel sbpTime;
		private System.Windows.Forms.Timer tmrDesc;
		private System.Windows.Forms.StatusBarPanel sbpDate;
		private System.Windows.Forms.ToolBarButton toolBarButton1;
		private System.Windows.Forms.StatusBarPanel sbpName;
		private System.Windows.Forms.Label lblNote;
		private SolidBrush[] m_aBrushes=new SolidBrush []
		{
			new SolidBrush(Color.Blue),			
			new SolidBrush(Color.Red),
			new SolidBrush(Color.Green)
		};

		public frmCuttingEars()
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
				if (components != null) 
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
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(frmCuttingEars));
			this.btnClose = new System.Windows.Forms.Button();
			this.btnCut = new System.Windows.Forms.Button();
			this.cmnuCuttingEar = new System.Windows.Forms.ContextMenu();
			this.cmnuDrawPolygon = new System.Windows.Forms.MenuItem();
			this.cmnCut = new System.Windows.Forms.MenuItem();
			this.cmnuClean = new System.Windows.Forms.MenuItem();
			this.cmnuClose = new System.Windows.Forms.MenuItem();
			this.btnDraw = new System.Windows.Forms.Button();
			this.gbPolygon = new System.Windows.Forms.GroupBox();
			this.rdoTestData = new System.Windows.Forms.RadioButton();
			this.rdoSelect = new System.Windows.Forms.RadioButton();
			this.pnlDraw = new System.Windows.Forms.Panel();
			this.tbCuttingEar = new System.Windows.Forms.ToolBar();
			this.toolBarButton1 = new System.Windows.Forms.ToolBarButton();
			this.tbbDraw = new System.Windows.Forms.ToolBarButton();
			this.tbbCut = new System.Windows.Forms.ToolBarButton();
			this.tbbClean = new System.Windows.Forms.ToolBarButton();
			this.tbbSeparator = new System.Windows.Forms.ToolBarButton();
			this.tbbClose = new System.Windows.Forms.ToolBarButton();
			this.ilToolbar = new System.Windows.Forms.ImageList(this.components);
			this.mnuCuttingEar = new System.Windows.Forms.MainMenu();
			this.mnuPolygon = new System.Windows.Forms.MenuItem();
			this.mnuTestData = new System.Windows.Forms.MenuItem();
			this.mnuMouseClick = new System.Windows.Forms.MenuItem();
			this.mnuDraw = new System.Windows.Forms.MenuItem();
			this.mnuDrawPolygon = new System.Windows.Forms.MenuItem();
			this.mnuEar = new System.Windows.Forms.MenuItem();
			this.mnuRedraw = new System.Windows.Forms.MenuItem();
			this.mnuHelp = new System.Windows.Forms.MenuItem();
			this.mnuSimplePolygon = new System.Windows.Forms.MenuItem();
			this.mnuAbout = new System.Windows.Forms.MenuItem();
			this.sbDesc = new System.Windows.Forms.StatusBar();
			this.sbpDes = new System.Windows.Forms.StatusBarPanel();
			this.sbpName = new System.Windows.Forms.StatusBarPanel();
			this.sbpDate = new System.Windows.Forms.StatusBarPanel();
			this.sbpTime = new System.Windows.Forms.StatusBarPanel();
			this.btnClear = new System.Windows.Forms.Button();
			this.ttpPolygon = new System.Windows.Forms.ToolTip(this.components);
			this.lblDesc = new System.Windows.Forms.Label();
			this.tmrDesc = new System.Windows.Forms.Timer(this.components);
			this.lblNote = new System.Windows.Forms.Label();
			this.gbPolygon.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.sbpDes)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.sbpName)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.sbpDate)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.sbpTime)).BeginInit();
			this.SuspendLayout();
			// 
			// btnClose
			// 
			this.btnClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.btnClose.Location = new System.Drawing.Point(432, 223);
			this.btnClose.Name = "btnClose";
			this.btnClose.Size = new System.Drawing.Size(136, 32);
			this.btnClose.TabIndex = 4;
			this.btnClose.Text = "Clo&se";
			this.ttpPolygon.SetToolTip(this.btnClose, "Exit");
			this.btnClose.Click += new System.EventHandler(this.btnClose_Click);
			// 
			// btnCut
			// 
			this.btnCut.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.btnCut.Location = new System.Drawing.Point(432, 149);
			this.btnCut.Name = "btnCut";
			this.btnCut.Size = new System.Drawing.Size(136, 32);
			this.btnCut.TabIndex = 2;
			this.btnCut.Text = "&Cut Ear";
			this.ttpPolygon.SetToolTip(this.btnCut, "Cut polygon ears");
			this.btnCut.Click += new System.EventHandler(this.btnCut_Click);
			// 
			// cmnuCuttingEar
			// 
			this.cmnuCuttingEar.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						   this.cmnuDrawPolygon,
																						   this.cmnCut,
																						   this.cmnuClean,
																						   this.cmnuClose});
			// 
			// cmnuDrawPolygon
			// 
			this.cmnuDrawPolygon.Index = 0;
			this.cmnuDrawPolygon.Text = "Draw Polygon";
			this.cmnuDrawPolygon.Click += new System.EventHandler(this.btnDraw_Click);
			// 
			// cmnCut
			// 
			this.cmnCut.Index = 1;
			this.cmnCut.Text = "Cut Ear";
			this.cmnCut.Click += new System.EventHandler(this.btnCut_Click);
			// 
			// cmnuClean
			// 
			this.cmnuClean.Index = 2;
			this.cmnuClean.Text = "Clean Screen";
			this.cmnuClean.Click += new System.EventHandler(this.btnClear_Click);
			// 
			// cmnuClose
			// 
			this.cmnuClose.Index = 3;
			this.cmnuClose.Text = "Close";
			this.cmnuClose.Click += new System.EventHandler(this.btnClose_Click);
			// 
			// btnDraw
			// 
			this.btnDraw.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.btnDraw.Location = new System.Drawing.Point(432, 112);
			this.btnDraw.Name = "btnDraw";
			this.btnDraw.Size = new System.Drawing.Size(136, 32);
			this.btnDraw.TabIndex = 1;
			this.btnDraw.Text = "&Draw Polygon";
			this.ttpPolygon.SetToolTip(this.btnDraw, "Draw a polygon");
			this.btnDraw.Click += new System.EventHandler(this.btnDraw_Click);
			// 
			// gbPolygon
			// 
			this.gbPolygon.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.gbPolygon.Controls.Add(this.rdoTestData);
			this.gbPolygon.Controls.Add(this.rdoSelect);
			this.gbPolygon.Location = new System.Drawing.Point(432, 32);
			this.gbPolygon.Name = "gbPolygon";
			this.gbPolygon.Size = new System.Drawing.Size(136, 72);
			this.gbPolygon.TabIndex = 0;
			this.gbPolygon.TabStop = false;
			this.gbPolygon.Text = "Generate Polygon:";
			this.ttpPolygon.SetToolTip(this.gbPolygon, "Initialize a polygon");
			// 
			// rdoTestData
			// 
			this.rdoTestData.Checked = true;
			this.rdoTestData.Location = new System.Drawing.Point(8, 24);
			this.rdoTestData.Name = "rdoTestData";
			this.rdoTestData.Size = new System.Drawing.Size(104, 16);
			this.rdoTestData.TabIndex = 1;
			this.rdoTestData.TabStop = true;
			this.rdoTestData.Text = "Using test data";
			// 
			// rdoSelect
			// 
			this.rdoSelect.Location = new System.Drawing.Point(8, 40);
			this.rdoSelect.Name = "rdoSelect";
			this.rdoSelect.Size = new System.Drawing.Size(120, 24);
			this.rdoSelect.TabIndex = 0;
			this.rdoSelect.Text = "Using mouse click";
			// 
			// pnlDraw
			// 
			this.pnlDraw.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.pnlDraw.AutoScroll = true;
			this.pnlDraw.BackColor = System.Drawing.Color.White;
			this.pnlDraw.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.pnlDraw.ContextMenu = this.cmnuCuttingEar;
			this.pnlDraw.Location = new System.Drawing.Point(8, 32);
			this.pnlDraw.Name = "pnlDraw";
			this.pnlDraw.Size = new System.Drawing.Size(416, 288);
			this.pnlDraw.TabIndex = 7;
			this.ttpPolygon.SetToolTip(this.pnlDraw, "Draw a polygon and cut ears...");
			this.pnlDraw.Click += new System.EventHandler(this.pnlDraw_Click);
			this.pnlDraw.MouseUp += new System.Windows.Forms.MouseEventHandler(this.frmCuttingEars_MouseUp);
			this.pnlDraw.Paint += new System.Windows.Forms.PaintEventHandler(this.pnlDraw_Paint);
			// 
			// tbCuttingEar
			// 
			this.tbCuttingEar.Buttons.AddRange(new System.Windows.Forms.ToolBarButton[] {
																							this.toolBarButton1,
																							this.tbbDraw,
																							this.tbbCut,
																							this.tbbClean,
																							this.tbbSeparator,
																							this.tbbClose});
			this.tbCuttingEar.ButtonSize = new System.Drawing.Size(18, 18);
			this.tbCuttingEar.DropDownArrows = true;
			this.tbCuttingEar.ImageList = this.ilToolbar;
			this.tbCuttingEar.Location = new System.Drawing.Point(0, 0);
			this.tbCuttingEar.Name = "tbCuttingEar";
			this.tbCuttingEar.ShowToolTips = true;
			this.tbCuttingEar.Size = new System.Drawing.Size(576, 28);
			this.tbCuttingEar.TabIndex = 8;
			this.tbCuttingEar.ButtonClick += new System.Windows.Forms.ToolBarButtonClickEventHandler(this.tbCuttingEar_ButtonClick);
			// 
			// toolBarButton1
			// 
			this.toolBarButton1.Style = System.Windows.Forms.ToolBarButtonStyle.Separator;
			// 
			// tbbDraw
			// 
			this.tbbDraw.ImageIndex = 0;
			this.tbbDraw.Text = "Draw";
			this.tbbDraw.ToolTipText = "Draw a polygon";
			// 
			// tbbCut
			// 
			this.tbbCut.ImageIndex = 1;
			this.tbbCut.Text = "Cut";
			this.tbbCut.ToolTipText = "Cut polygon ears";
			// 
			// tbbClean
			// 
			this.tbbClean.ImageIndex = 2;
			this.tbbClean.Text = "Clean";
			this.tbbClean.ToolTipText = "Reset";
			// 
			// tbbSeparator
			// 
			this.tbbSeparator.Style = System.Windows.Forms.ToolBarButtonStyle.Separator;
			this.tbbSeparator.Text = "Separator";
			// 
			// tbbClose
			// 
			this.tbbClose.ImageIndex = 3;
			this.tbbClose.Text = "Close";
			this.tbbClose.ToolTipText = "Exit";
			// 
			// ilToolbar
			// 
			this.ilToolbar.ImageSize = new System.Drawing.Size(16, 16);
			this.ilToolbar.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("ilToolbar.ImageStream")));
			this.ilToolbar.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// mnuCuttingEar
			// 
			this.mnuCuttingEar.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						  this.mnuPolygon,
																						  this.mnuDraw,
																						  this.mnuHelp});
			// 
			// mnuPolygon
			// 
			this.mnuPolygon.Index = 0;
			this.mnuPolygon.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					   this.mnuTestData,
																					   this.mnuMouseClick});
			this.mnuPolygon.Text = "&Polygon";
			this.mnuPolygon.Popup += new System.EventHandler(this.mnuPolygon_Popup);
			// 
			// mnuTestData
			// 
			this.mnuTestData.Index = 0;
			this.mnuTestData.RadioCheck = true;
			this.mnuTestData.Shortcut = System.Windows.Forms.Shortcut.CtrlT;
			this.mnuTestData.Text = "Using Test Data";
			this.mnuTestData.Popup += new System.EventHandler(this.mnuTestData_Popup);
			this.mnuTestData.Click += new System.EventHandler(this.mnuTestData_Click);
			// 
			// mnuMouseClick
			// 
			this.mnuMouseClick.Index = 1;
			this.mnuMouseClick.RadioCheck = true;
			this.mnuMouseClick.Shortcut = System.Windows.Forms.Shortcut.CtrlM;
			this.mnuMouseClick.Text = "Using Mouse Click";
			this.mnuMouseClick.Popup += new System.EventHandler(this.mnuMouseClick_Popup);
			this.mnuMouseClick.Click += new System.EventHandler(this.mnuMouseClick_Click);
			// 
			// mnuDraw
			// 
			this.mnuDraw.Index = 1;
			this.mnuDraw.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					this.mnuDrawPolygon,
																					this.mnuEar,
																					this.mnuRedraw});
			this.mnuDraw.Text = "Dra&w";
			// 
			// mnuDrawPolygon
			// 
			this.mnuDrawPolygon.Index = 0;
			this.mnuDrawPolygon.Shortcut = System.Windows.Forms.Shortcut.CtrlG;
			this.mnuDrawPolygon.Text = "&Polygon";
			this.mnuDrawPolygon.Click += new System.EventHandler(this.btnDraw_Click);
			// 
			// mnuEar
			// 
			this.mnuEar.Index = 1;
			this.mnuEar.Shortcut = System.Windows.Forms.Shortcut.CtrlR;
			this.mnuEar.Text = "&Ears && Polygon";
			this.mnuEar.Click += new System.EventHandler(this.btnCut_Click);
			// 
			// mnuRedraw
			// 
			this.mnuRedraw.Index = 2;
			this.mnuRedraw.Shortcut = System.Windows.Forms.Shortcut.CtrlW;
			this.mnuRedraw.Text = "&Redraw";
			this.mnuRedraw.Click += new System.EventHandler(this.btnClear_Click);
			// 
			// mnuHelp
			// 
			this.mnuHelp.Index = 2;
			this.mnuHelp.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					this.mnuSimplePolygon,
																					this.mnuAbout});
			this.mnuHelp.Text = "&Help";
			// 
			// mnuSimplePolygon
			// 
			this.mnuSimplePolygon.Index = 0;
			this.mnuSimplePolygon.Text = "&Simple Polygon";
			this.mnuSimplePolygon.Click += new System.EventHandler(this.mnuSimplePolygon_Click);
			// 
			// mnuAbout
			// 
			this.mnuAbout.Index = 1;
			this.mnuAbout.Text = "&About";
			this.mnuAbout.Click += new System.EventHandler(this.mnuAbout_Click);
			// 
			// sbDesc
			// 
			this.sbDesc.Location = new System.Drawing.Point(0, 331);
			this.sbDesc.Name = "sbDesc";
			this.sbDesc.Panels.AddRange(new System.Windows.Forms.StatusBarPanel[] {
																					  this.sbpDes,
																					  this.sbpName,
																					  this.sbpDate,
																					  this.sbpTime});
			this.sbDesc.ShowPanels = true;
			this.sbDesc.Size = new System.Drawing.Size(576, 22);
			this.sbDesc.TabIndex = 9;
			this.ttpPolygon.SetToolTip(this.sbDesc, "Demo: cutting polygon ears...");
			// 
			// sbpDes
			// 
			this.sbpDes.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents;
			this.sbpDes.Width = 10;
			// 
			// sbpName
			// 
			this.sbpName.Alignment = System.Windows.Forms.HorizontalAlignment.Center;
			this.sbpName.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents;
			this.sbpName.Text = "  F.S  ";
			this.sbpName.Width = 38;
			// 
			// sbpDate
			// 
			this.sbpDate.Alignment = System.Windows.Forms.HorizontalAlignment.Right;
			this.sbpDate.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents;
			this.sbpDate.Width = 10;
			// 
			// sbpTime
			// 
			this.sbpTime.Alignment = System.Windows.Forms.HorizontalAlignment.Right;
			this.sbpTime.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents;
			this.sbpTime.Text = "statusBarPanel3";
			this.sbpTime.Width = 97;
			// 
			// btnClear
			// 
			this.btnClear.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.btnClear.Location = new System.Drawing.Point(432, 186);
			this.btnClear.Name = "btnClear";
			this.btnClear.Size = new System.Drawing.Size(136, 32);
			this.btnClear.TabIndex = 3;
			this.btnClear.Text = "C&lean Screen";
			this.ttpPolygon.SetToolTip(this.btnClear, "Reset");
			this.btnClear.Click += new System.EventHandler(this.btnClear_Click);
			// 
			// ttpPolygon
			// 
			this.ttpPolygon.AutoPopDelay = 5000;
			this.ttpPolygon.InitialDelay = 500;
			this.ttpPolygon.ReshowDelay = 100;
			// 
			// lblDesc
			// 
			this.lblDesc.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.lblDesc.AutoSize = true;
			this.lblDesc.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblDesc.Location = new System.Drawing.Point(432, 296);
			this.lblDesc.Name = "lblDesc";
			this.lblDesc.Size = new System.Drawing.Size(30, 16);
			this.lblDesc.TabIndex = 10;
			this.lblDesc.Text = "Desc";
			// 
			// tmrDesc
			// 
			this.tmrDesc.Enabled = true;
			this.tmrDesc.Interval = 1000;
			this.tmrDesc.Tick += new System.EventHandler(this.tmrDesc_Tick);
			// 
			// lblNote
			// 
			this.lblNote.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.lblNote.AutoSize = true;
			this.lblNote.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblNote.Location = new System.Drawing.Point(432, 272);
			this.lblNote.Name = "lblNote";
			this.lblNote.Size = new System.Drawing.Size(31, 16);
			this.lblNote.TabIndex = 11;
			this.lblNote.Text = "Note:";
			// 
			// frmCuttingEars
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(576, 353);
			this.Controls.Add(this.lblNote);
			this.Controls.Add(this.lblDesc);
			this.Controls.Add(this.btnClear);
			this.Controls.Add(this.btnDraw);
			this.Controls.Add(this.gbPolygon);
			this.Controls.Add(this.pnlDraw);
			this.Controls.Add(this.tbCuttingEar);
			this.Controls.Add(this.sbDesc);
			this.Controls.Add(this.btnClose);
			this.Controls.Add(this.btnCut);
			this.Menu = this.mnuCuttingEar;
			this.Name = "frmCuttingEars";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
			this.Text = "Cut Convex Polygon Ears";
			this.Load += new System.EventHandler(this.frmCuttingEars_Load);
			this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.frmCuttingEars_MouseUp);
			this.Paint += new System.Windows.Forms.PaintEventHandler(this.frmCuttingEars_Paint);
			this.gbPolygon.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.sbpDes)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.sbpName)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.sbpDate)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.sbpTime)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			frmCuttingEars frmInterface=new frmCuttingEars();
			frmInterface.ResizeRedraw=true;
			Application.Run(frmInterface);
		}

		private void ReadDefaultData()
		{
			/*
			m_aPolygonPts=new Point[4];
			
			m_aPolygonPts[0]=new Point(50,70);
			m_aPolygonPts[1]=new Point(180,280);
			
			m_aPolygonPts[2]=new Point(260,80);
			m_aPolygonPts[3]=new Point(240,20);
			*/
			
			m_aPolygonPts=new Point[6];
			
			m_aPolygonPts[0]=new Point(50,70);
			m_aPolygonPts[1]=new Point(180,280);
			m_aPolygonPts[2]=new Point(140,140);
			
			m_aPolygonPts[3]=new Point(140,80);
			m_aPolygonPts[4]=new Point(190,50);
			m_aPolygonPts[5]=new Point(240,20);
			
			/*
			m_aPolygonPts=new Point[5];
			
			m_aPolygonPts[0]=new Point(50,50);
			m_aPolygonPts[1]=new Point(50,250);
			m_aPolygonPts[2]=new Point(160,200);
			
			m_aPolygonPts[3]=new Point(300,250);
			m_aPolygonPts[4]=new Point(300,50);
			*/
		}

		private void ReadSelectedData()
		{
			if (m_alSelectedPts.Count<3)
				MessageBox.Show("Please using mouse to pick at least 3 points!",
					"How to initialize a Polygon", 
					MessageBoxButtons.OK, MessageBoxIcon.Information);
			else
			{
				m_aPolygonPts=new Point[m_alSelectedPts.Count];
				for (int i=0; i<m_alSelectedPts.Count; i++)
				{
					m_aPolygonPts[i]=(Point)m_alSelectedPts[i];
				}
			}
		}

		private void btnClear_Click(object sender, System.EventArgs e)
		{
			m_aPolygonPts=new Point[0];
			m_alSelectedPts.Clear();
			pnlDraw.CreateGraphics().Clear(pnlDraw.BackColor);
			Invalidate();
		}

		public void DrawEarsPolygon(CPolygonShape cutPolygon)
		{
			//draw polygon and convex polygonn
			if (cutPolygon==null)
				return;

			for (int i=0; i <cutPolygon.NumberOfPolygons; i++)
			{
				int nPoints=cutPolygon.Polygons(i).Length;
				Point[] tempArray=new Point[nPoints];	
				for (int j=0; j<nPoints; j++)
				{
					tempArray[j].X= (int)cutPolygon.Polygons(i)[j].X;
					tempArray[j].Y= (int)cutPolygon.Polygons(i)[j].Y;
				}

				Graphics gfx=pnlDraw.CreateGraphics();  
				int nBrush = i % 3;
				gfx.FillPolygon(m_aBrushes[nBrush], tempArray);
				Invalidate();
			}
		}
		
		private void btnCut_Click(object sender, System.EventArgs e)
		{			
			DrawOuterPolygon();

			int nVertices=m_aPolygonPts.Length;
			if (nVertices<=0)
				return;

			CPoint2D[] vertices=new CPoint2D[nVertices];
			for (int i=0; i<nVertices; i++)
			{
				vertices[i]=new CPoint2D(m_aPolygonPts[i].X,
					m_aPolygonPts[i].Y);
			}
			
			CPolygonShape cutPolygon =  new CPolygonShape(vertices);
			cutPolygon.CutEar();

			DrawEarsPolygon(cutPolygon);
		}


		private void btnClose_Click(object sender, System.EventArgs e)
		{
			this.Close();
		}

		private void DrawOuterPolygon()
		{
			if (rdoTestData.Checked)
				ReadDefaultData();
			else if (rdoSelect.Checked)
				ReadSelectedData();
			else
				return;
			
			if (m_aPolygonPts!=null) 
			{
				if (m_aPolygonPts.Length>0)
				{
					Graphics gfx=this.pnlDraw.CreateGraphics();
					gfx.DrawPolygon(m_PolygonPen, m_aPolygonPts);
				}
			}
		}

		private void btnDraw_Click(object sender, System.EventArgs e)
		{
			DrawOuterPolygon();	
		}

		private void pnlDraw_Click(object sender, System.EventArgs e){}
		private void frmCuttingEars_Paint(object sender, 
			System.Windows.Forms.PaintEventArgs e){}

		private void frmCuttingEars_Load(object sender, System.EventArgs e)
		{
			tbbClean.Tag=btnClear;
			tbbDraw.Tag=btnDraw;
			tbbCut.Tag=btnCut;
			tbbClose.Tag=btnClose;

			lblDesc.Text="Use Simple Polygon only.";
			
			sbpDes.Text= "Developing and Implementing Applications with Visual C#";
		}
		
		

		private void pnlDraw_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
		{
			if (m_aPolygonPts!=null)
			{
				if (m_aPolygonPts.Length>0)
				{
					Graphics gfx=e.Graphics;
					gfx.DrawPolygon(	m_PolygonPen, m_aPolygonPts);			
				}
			}
		}
	
		private void DrawCross(int x, int y)
		{
			Graphics gfx=pnlDraw.CreateGraphics();
			Point startPt=new Point(x-2,y);
			Point endPt=new Point(x+2,y);
			gfx.DrawLine(Pens.Red,startPt, endPt);

			startPt=new Point(x,y-2);
			endPt=new Point(x,y+2);
			gfx.DrawLine(Pens.Red,startPt, endPt);
		}
		
		private void frmCuttingEars_MouseUp(object sender, 
			System.Windows.Forms.MouseEventArgs e)
		{
			if (!rdoSelect.Checked)
				return;

			Point clickedPt=new Point(e.X, e.Y);
			m_alSelectedPts.Add(clickedPt);
			DrawCross(e.X,e.Y);
		}

		private void mnuTestData_Click(object sender, System.EventArgs e)
		{
			rdoTestData.Checked=true;
		}

		private void mnuMouseClick_Click(object sender, System.EventArgs e)
		{
			rdoSelect.Checked=true;
		}

		private void mnuMouseClick_Popup(object sender, System.EventArgs e)
		{

		}

		private void mnuTestData_Popup(object sender, System.EventArgs e)
		{

		}

		private void mnuPolygon_Popup(object sender, System.EventArgs e)
		{
			mnuMouseClick.Checked=false;
			if (rdoSelect.Checked)
			mnuMouseClick.Checked=true;

			mnuTestData.Checked=false;
			if (rdoTestData.Checked)
			mnuTestData.Checked=true;
		}

		

		private void mnuSimplePolygon_Click(object sender, System.EventArgs e)
		{
			Form frm=new frmHelp_SimplePolygon();
			//frm.Show();
			frm.ShowDialog(this);
		}

		private void mnuAbout_Click(object sender, System.EventArgs e)
		{
			Form frm=new frmHelp_About();
			//frm.Show();
			frm.ShowDialog(this);
		}

		private void tbCuttingEar_ButtonClick(object sender, System.Windows.Forms.ToolBarButtonClickEventArgs e)
		{
			ToolBarButton btnTool=(ToolBarButton)e.Button;
			Button btn=(Button)btnTool.Tag;
			btn.PerformClick();
		}

		private void tmrDesc_Tick(object sender, System.EventArgs e)
		{
			DateTime dtNow=DateTime.Now;
			sbpTime.Text=dtNow.ToShortTimeString();
			sbpDate.Text=dtNow.ToLongDateString();
		}
	}
}
