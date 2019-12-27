using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace LeapfrogEditor
{
   public class LfPolygonViewModel : LfShapeViewModel
   {
      #region Declarations

      private ObservableCollection<LfDragablePointViewModel> _pointVms = new ObservableCollection<LfDragablePointViewModel>();
      private ObservableCollection<TriangleViewModel> _triangles = new ObservableCollection<TriangleViewModel>();

      #endregion

      #region Constructors

      public LfPolygonViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
      }

      #endregion

      #region Properties

      public LfPolygon LocalModelObject
      {
         get { return (LfPolygon)ModelObject; }
      }

      public PointCollection Points
      {
         get
         {
            PointCollection p = new PointCollection();

            foreach (LfDragablePointViewModel dp in PointVms)
            {
               p.Add(new Point(dp.PosX, dp.PosY));
            }

            return p;
         }
         set
         { }

      }

      public PointCollection AbsPoints
      {
         get
         {
            PointCollection p = new PointCollection();

            foreach (LfDragablePointViewModel dp in PointVms)
            {
               p.Add(new Point(dp.AbsPosX, dp.AbsPosY));
            }

            return p;
         }
         set
         { }

      }

      public ObservableCollection<LfDragablePointViewModel> PointVms
      {
         get { return _pointVms; }
         set { _pointVms = value; }
      }

      public ObservableCollection<TriangleViewModel> Triangles
      {
         get { return _triangles; }
         set { _triangles = value; }
      }

      #endregion

      #region protected Methods

      protected override void DeselectAllChildren()
      {
         DeselectAllPoints();
      }

      #endregion


      #region public Methods

      public override void InvalidateAll()
      {
         OnPropertyChanged("");

         foreach (LfDragablePointViewModel dp in PointVms)
         {
            dp.OnPropertyChanged("");
         }
      }

      public LfDragablePointViewModel AddPoint(Point point)
      {
         if (LocalModelObject == null) return null;

         LfDragablePoint np = new LfDragablePoint(GetFreeVertexIndex(), point.X, point.Y);
         LfDragablePointViewModel newPoint = new LfDragablePointViewModel(this, ParentVm, MainVm, this, np);
         PointVms.Add(newPoint);
         LocalModelObject.AddPoint(newPoint.ModelObject);

         InvalidateAll();
         ParentVm.OnPropertyChanged("");

         return newPoint;
      }

      public void RemovePoint(LfDragablePointViewModel point)
      {
         if (LocalModelObject == null) return;

         PointVms.Remove(point);
         LocalModelObject.RemovePoint(point.ModelObject);

         InvalidateAll();
         ParentVm.OnPropertyChanged("");
      }

      public LfDragablePointViewModel InsertPoint(Point insertMe, LfDragablePointViewModel insertBeforeMe)
      {
         if (LocalModelObject == null) return null;

         LfDragablePoint np = new LfDragablePoint(GetFreeVertexIndex(), insertMe.X, insertMe.Y);
         LfDragablePointViewModel newPoint = new LfDragablePointViewModel(this, ParentVm, MainVm, this, np);

         int index = 0;

         if (insertBeforeMe != null)
         {
            index = PointVms.IndexOf(insertBeforeMe);
         }

         if (index >= 0)
         {
            PointVms.Insert(index, newPoint);
         }

         if (insertBeforeMe != null)
         {
            LocalModelObject.InsertPoint(newPoint.ModelObject, insertBeforeMe.ModelObject);
         }
         else
         {
            LocalModelObject.InsertPoint(newPoint.ModelObject, null);
         }

         InvalidateAll();
         ParentVm.OnPropertyChanged("");

         return newPoint;
      }

      public void DeselectAllPoints()
      {
         foreach (LfDragablePointViewModel dp in PointVms)
         {
            dp.IsSelected = false;
         }
      }

      public void GenerateTriangles()
      {
         GeometryUtility.CPoint2D[] polyVertices = new GeometryUtility.CPoint2D[PointVms.Count];

         int i = 0;

         foreach (LfDragablePointViewModel dpvm in PointVms)
         {
            // Before triangles are generated we must reorder the identities of the 
            // vertices to make them consequtive
            dpvm.Id = (uint)(i + 1);

            polyVertices[i] = new GeometryUtility.CPoint2D(dpvm.PosX, dpvm.PosY, dpvm.Id);

            i++;
         }

         PolygonCuttingEar.CPolygonShape poly = new PolygonCuttingEar.CPolygonShape(polyVertices);

         poly.CutEar();

         if (LocalModelObject != null)
         {
            LocalModelObject.Triangles.Clear();
            _triangles.Clear();

            for (int n = 0; n < poly.NumberOfPolygons; n++)
            {
               Triangle nt = new Triangle((uint)n+1, poly.Polygons(n)[0].Id, poly.Polygons(n)[1].Id, poly.Polygons(n)[2].Id);
               LocalModelObject.Triangles.Add(nt);

               TriangleViewModel ntvm = new TriangleViewModel(this, nt);
               _triangles.Add(ntvm);
            }
         }
      }

      public LfDragablePointViewModel GetTrianglePoint(uint pointId)
      {
         foreach (LfDragablePointViewModel dpvm in PointVms)
         {
            if (dpvm.Id == pointId)
            {
               return dpvm;
            }
         }

         return null;
      }

      #endregion


      #region private Methods

      private uint GetFreeVertexIndex()
      {
         bool freeIdFound = true;
         uint id = 1;

         do
         {
            freeIdFound = true;

            foreach (LfDragablePointViewModel dpvm in PointVms)
            {
               if (id == dpvm.Id)
               {
                  id++;
                  freeIdFound = false;
                  break;
               }
            }
         }
         while (!freeIdFound);

         return id;
      }

      #endregion




      #region protected Methods

      protected override Rect GetBoundingBox()
      {
         double l = double.MaxValue;
         double r = double.MinValue;
         double t = double.MaxValue;
         double b = double.MinValue;

         foreach (LfDragablePointViewModel dp in _pointVms)
         {
            // Convert point according to angle
            Point rtp = CoordinateTransformations.RotatedPointFromLocal(new Point(dp.PosX, dp.PosY), Angle);
            

            if (rtp.X < l)
            {
               l = rtp.X;
            }

            if (rtp.X > r)
            {
               r = rtp.X;
            }

            if (rtp.Y < t)
            {
               t = rtp.Y;
            }

            if (rtp.Y > b)
            {
               b = rtp.Y;
            }
         }
         Rect tr = new Rect(new Point(l, t), new Point(r, b));

         return tr;
      }

      #endregion

   }
}
