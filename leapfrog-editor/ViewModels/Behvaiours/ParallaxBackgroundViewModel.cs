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
   public class ParallaxBackgroundViewModel : CompoundShapesObjectViewModel
   {
      #region Declarations

      private ParallaxBackground _modelObject;

      #endregion

      #region Constructors

      public ParallaxBackgroundViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         ParallaxBackground modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, modelObject, enabled)
      {
         ModelObject = modelObject;

         // This class builds its view model from the model object automatically
         // since there is no linkage between shapes and joints and systems
         // The CompoundObjectViewModel does however use a BuildViewModel method
         // which is called explicitly
         foreach (LfSpriteBox sb in ModelObject.SpriteBoxes)
         {
            LfSpriteBoxViewModel sbvm = new LfSpriteBoxViewModel(this, parentVm, mainVm, sb, false);
            ShapeCollection.Shapes.Add(sbvm);
         }

         foreach (LfSpritePolygon sp in ModelObject.SpritePolygons)
         {
            LfSpritePolygonViewModel spvm = new LfSpritePolygonViewModel(this, parentVm, mainVm, sp, false);

            foreach (LfDragablePoint dragPoint in sp.Points)
            {
               LfDragablePointViewModel dragPointVm = new LfDragablePointViewModel(spvm, parentVm, MainVm, spvm, dragPoint, enabled);
               spvm.PointVms.Add(dragPointVm);
            }

            ShapeCollection.Shapes.Add(spvm);
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

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      override public LfShapeViewModel AddShape(LeftClickState shapeType, Point position)
      {
         LfShape newShape = null;
         LfShapeViewModel newShapeVm = null;


         if (shapeType == LeftClickState.spriteBox)
         {
            newShape = new LfSpriteBox();
            newShapeVm = new LfSpriteBoxViewModel(ShapeCollection, null, MainVm, (LfSpriteBox)newShape);
            ModelObject.SpriteBoxes.Add((LfSpriteBox)newShape);
         }
         else if (shapeType == LeftClickState.spritePolygon)
         {
            newShape = new LfSpritePolygon();
            newShapeVm = new LfSpritePolygonViewModel(ShapeCollection, null, MainVm, (LfSpritePolygon)newShape);
            ModelObject.SpritePolygons.Add((LfSpritePolygon)newShape);
         }

         if (newShapeVm != null)
         {
            newShapeVm.PosX = position.X;
            newShapeVm.PosY = position.Y;
         }

         ShapeCollection.Shapes.Add(newShapeVm);

         return newShapeVm;

      }

      override public void RemoveShape(LfShapeViewModel svm)
      {
         // Remove the shape model
         ModelObject.RemoveShape(svm.ModelObject);

         // Remove the shape viewmodel from this
         ShapeCollection.Shapes.Remove(svm);

         OnPropertyChanged("");
      }

      #endregion
   }
}
