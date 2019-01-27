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
   public class TriangleViewModel : MicroMvvm.ViewModelBase
   {
      #region Declarations

      private Triangle _modelObject;
      private LfPolygonViewModel _parent;

      private ObservableCollection<LfDragablePointViewModel> _pointVms = new ObservableCollection<LfDragablePointViewModel>();

      #endregion

      #region Constructors

      public TriangleViewModel(LfPolygonViewModel parent, Triangle modelObject)
      {
         Parent = parent;
         ModelObject = modelObject;
         BuildPoints();
      }

      #endregion

      #region Properties

      public Triangle ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            OnPropertyChanged("");
         }
      }

      public LfPolygonViewModel Parent
      {
         get { return _parent; }
         set
         {
            _parent = value;
            OnPropertyChanged("");
         }
      }

      public ObservableCollection<LfDragablePointViewModel> PointVms
      {
         get { return _pointVms; }
         set { _pointVms = value; }
      }

      public uint Id
      {
         get
         {
            if (ModelObject == null) return 0;

            return ModelObject.Id;
         }
         set
         {
            if (ModelObject == null) return;

            ModelObject.Id = value;
            OnPropertyChanged("Id");
         }
      }

      #endregion

      #region private  methods

      private void BuildPoints()
      {
         PointVms.Clear();

         PointVms.Add(Parent.GetTrianglePoint(ModelObject.V1));
         PointVms.Add(Parent.GetTrianglePoint(ModelObject.V2));
         PointVms.Add(Parent.GetTrianglePoint(ModelObject.V3));
      }

      #endregion
   }
}
