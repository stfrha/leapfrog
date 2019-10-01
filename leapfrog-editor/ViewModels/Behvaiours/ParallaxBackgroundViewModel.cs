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
   public class ParallaxBackgroundViewModel : TreeViewViewModel
   {
      #region Declarations

      private ParallaxBackground _modelObject;

      private CompositeCollection _shapes;

      #endregion

      #region Constructors

      public ParallaxBackgroundViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         ParallaxBackground modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;

         _shapes = new CompositeCollection()
         {
            new CollectionContainer { Collection = new ObservableCollection<LfSpriteBoxViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfSpritePolygonViewModel>() },
         };

         foreach (LfSpriteBox sb in ModelObject.SpriteBoxes)
         {
            LfSpriteBoxViewModel sbvm = new LfSpriteBoxViewModel(this, parentVm, mainVm, sb, enabled);
            _shapes.Add(sbvm);
         }

         foreach (LfSpritePolygon sp in ModelObject.SpritePolygons)
         {
            LfSpritePolygonViewModel spvm = new LfSpritePolygonViewModel(this, parentVm, mainVm, sp, enabled);

            foreach (LfDragablePoint dragPoint in sp.Points)
            {
               LfDragablePointViewModel dragPointVm = new LfDragablePointViewModel(spvm, parentVm, MainVm, spvm, dragPoint, enabled);
               spvm.PointVms.Add(dragPointVm);
            }

            _shapes.Add(spvm);
         }
      }

      #endregion

      #region Properties

      public ParallaxBackground ModelObject
      {
         get
         {
            if (_modelObject == null) return null;
            return _modelObject;
         }
         set
         {
            _modelObject = value;
            OnPropertyChanged("");
         }
      }

      public double ParallaxAmount
      {
         get
         {
            if (_modelObject == null) return 0;
            return _modelObject.ParallaxAmount;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.ParallaxAmount = value;
            OnPropertyChanged("");
         }
      }

      public string BackgroundName
      {
         get
         {
            if (_modelObject == null) return "Model object is NULL";
            return _modelObject.BackgroundName;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.BackgroundName = value;
            OnPropertyChanged("");
         }
      }

      public CompositeCollection Shapes
      {
         get {
            return _shapes;
         }
         set { _shapes = value; }
      }

      #endregion

      #region protected Methods

      protected virtual void DeselectAllChildren()
      {
         // Only polygon shapes has children (so far), se we do nothing here
      }

      #endregion

      #region public Methods

      //public virtual void InvalidateAll()
      //{
      //   OnPropertyChanged("");
      //}

      #endregion
   }
}
