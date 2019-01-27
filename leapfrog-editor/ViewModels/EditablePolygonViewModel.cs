using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace LeapfrogEditor
{
   class EditablePolygonViewModel : MicroMvvm.ViewModelBase, IShapeInterface
   {
      #region Declarations

      private MainViewModel _mainVm;
      private EditablePolygon _polygonObject;
      private CompoundObjectViewModel _parent;
      private ObservableCollection<DragablePointViewModel> _pointVms = new ObservableCollection<DragablePointViewModel>();

      private bool _isSelected;

      #endregion

      #region Constructors

      public EditablePolygonViewModel(MainViewModel mainVm, CompoundObjectViewModel parent, EditablePolygon pointObject)
      {
         MainVm = mainVm;
         Parent = parent;
         PolygonObject = pointObject;
      }

      #endregion

      #region Properties

      public MainViewModel MainVm
      {
         get { return _mainVm; }
         set { _mainVm = value; }
      }

      public EditablePolygon PolygonObject
      {
         get { return _polygonObject; }
         set
         {
            _polygonObject = value;
            OnPropertyChanged("");
         }
      }

      public CompoundObjectViewModel Parent
      {
         get { return _parent; }
         set
         {
            _parent = value;
            OnPropertyChanged("");
         }
      }

      public PointCollection Points
      {
         get
         {
            PointCollection p = new PointCollection();

            foreach (DragablePointViewModel dp in PointVms)
            {
               p.Add(new Point(dp.PosX, dp.PosY));
            }

            return p;
         }
         set
         {

         }

      }

      public ObservableCollection<DragablePointViewModel> PointVms
      {
         get { return _pointVms; }
         set { _pointVms = value; }
      }

      public ObservableCollection<DragablePointViewModel> ClosedPointVms
      {
         get
         {
            // Copy points collection
            ObservableCollection<DragablePointViewModel> tc = new ObservableCollection<DragablePointViewModel>(_pointVms);

            // Add first item to end to get a closed path
            tc.Add(tc[0]);

            return tc;
         }
         set {}
      }

      public double PosX
      {
         get
         {
            return _polygonObject.PosX;
         }
         set
         {
            _polygonObject.PosX = value;
            OnPropertyChanged("PosX");
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = Parent;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.Parent;
            }
         }
      }

      public double PosY
      {
         get
         {
            return _polygonObject.PosY;
         }
         set
         {
            _polygonObject.PosY = value;
            OnPropertyChanged("PosY");
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = Parent;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.Parent;
            }
         }
      }

      public Rect BoundingBox
      {
         get
         {
            double l = double.MaxValue;
            double r = double.MinValue;
            double t = double.MaxValue;
            double b = double.MinValue;

            foreach (DragablePointViewModel dp in _pointVms)
            {
               if (dp.PosX < l)
               {
                  l = dp.PosX;
               }

               if (dp.PosX > r)
               {
                  r = dp.PosX;
               }

               if (dp.PosY < t)
               {
                  t = dp.PosY;
               }

               if (dp.PosY > b)
               {
                  b = dp.PosY;
               }
               //if (dp.PosX + PosX < l)
               //{
               //   l = dp.PosX + PosX;
               //}

               //if (dp.PosX + PosX > r)
               //{
               //   r = dp.PosX + PosX;
               //}

               //if (dp.PosY + PosY < t)
               //{
               //   t = dp.PosY + PosY;
               //}

               //if (dp.PosY + PosY > b)
               //{
               //   b = dp.PosY + PosY;
               //}
            }
            Rect tr = new Rect(new Point(l, t), new Point(r, b));
//            tr.Offset(new Vector(PosX, PosY));
            return tr;
         }
         set
         { }
      }

      public bool IsSelected
      {
         get { return _isSelected; }
         set
         {
            _isSelected = value;

            if (value == false)
            {
               foreach (DragablePointViewModel dp in PointVms)
               {
                  dp.IsSelected = false;
               }
            }

            OnPropertyChanged("IsSelected");
         }
      }

      #endregion

      #region public Methods

      public DragablePointViewModel AddPoint(Point point)
      {
         DragablePoint np = new DragablePoint(1, point.X, point.Y);
         DragablePointViewModel newPoint = new DragablePointViewModel(MainVm, this, np);
         PointVms.Add(newPoint);
         PolygonObject.AddPoint(newPoint.ModelObject);

         OnPropertyChanged("");
         Parent.OnPropertyChanged("");

         return newPoint;
      }

      public void RemovePoint(DragablePointViewModel point)
      {
         PointVms.Remove(point);
         PolygonObject.RemovePoint(point.ModelObject);
         OnPropertyChanged("ClosedPointVms");
         OnPropertyChanged("Points");
         OnPropertyChanged("");
         Parent.OnPropertyChanged("");
      }

      public DragablePointViewModel InsertPoint(Point insertMe, DragablePointViewModel insertBeforeMe)
      {
         DragablePoint np = new DragablePoint(1, insertMe.X - PosX, insertMe.Y - PosY);
         DragablePointViewModel newPoint = new DragablePointViewModel(MainVm, this, np);

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
            PolygonObject.InsertPoint(newPoint.ModelObject, insertBeforeMe.ModelObject);
         }
         else
         {
            PolygonObject.InsertPoint(newPoint.ModelObject, null);
         }

         OnPropertyChanged("");
         Parent.OnPropertyChanged("");

         return newPoint;
      }

      public void DeselectAllPoints()
      {
         foreach (DragablePointViewModel dp in PointVms)
         {
            dp.IsSelected = false;
         }
      }

      #endregion

   }
}
