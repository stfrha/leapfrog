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

      private ShapeCollectionViewModel _shapes;

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

      public ShapeCollectionViewModel ShapeCollection
      {
         get { return _shapes; }
         set
         {
            _shapes = value;
            OnPropertyChanged("ShapeCollection");
         }
      }


      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      public LfShapeViewModel AddShape(LeftClickState shapeType, Point position)
      {
         LfShape newShape = null;
         LfShapeViewModel newShapeVm = null;


         if (shapeType == LeftClickState.spriteBox)
         {
            newShape = new LfSpriteBox();
            newShapeVm = new LfSpriteBoxViewModel(ShapeCollection, this, MainVm, (LfSpriteBox)newShape);
            ModelObject.SpriteBoxes.Add((LfSpriteBox)newShape);
         }
         else if (shapeType == LeftClickState.spritePolygon)
         {
            newShape = new LfSpritePolygon();
            newShapeVm = new LfSpritePolygonViewModel(ShapeCollection, this, MainVm, (LfSpritePolygon)newShape);
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

      public void RemoveShape(LfShapeViewModel svm)
      {
         // Remove the shape model
         ModelObject.RemoveShape(svm.ModelObject);

         // Remove the shape viewmodel from this
         ShapeCollection.Shapes.Remove(svm);

         // If there are no more shapes in the CO, remove the CO
         if (ShapeCollection.Shapes.Count == 0)
         {
            //ParentVm.StateChildObjects.Remove(this);
            //ParentVm.ModelObject.ChildObjectRefs(this.ChildObjectOfParent)
         }

         OnPropertyChanged("");
      }

      public LfDragablePointViewModel AddPoint(LfPolygonViewModel polyVm, Point position)
      {
         LfPolygonViewModel newPolygon = polyVm;
         Point parentObjectOrigo = new Point(newPolygon.ParentVm.PosX, newPolygon.ParentVm.PosY);
         Point shapeOrigo = new Point(newPolygon.PosX, newPolygon.PosY);
         shapeOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
         Point localClickPoint = new Point();
         localClickPoint = (Point)(position - shapeOrigo);

         LfDragablePointViewModel newPoint = newPolygon.AddPoint(localClickPoint);

         return newPoint;
      }

      public LfShapeViewModel FindShape(string name, ShapeCollectionViewModel shapes)
      {
         foreach (object o in shapes.Shapes)
         {
            if (o is LfShapeViewModel)
            {
               LfShapeViewModel shape = (LfShapeViewModel)o;

               if (shape.Name == name)
               {
                  return shape;
               }
            }
         }

         return null;
      }


      public void DeselectAllChildren()
      {
         if ((ShapeCollection != null) && (ShapeCollection.Shapes != null))
         {
            foreach (object o in ShapeCollection.Shapes)
            {
               if (o is LfPolygonViewModel)
               {
                  LfPolygonViewModel pvm = o as LfPolygonViewModel;

                  pvm.DeselectAllPoints();
                  pvm.IsSelected = false;
               }
               else if (o is LfShapeViewModel)
               {
                  LfShapeViewModel shape = o as LfShapeViewModel;

                  shape.IsSelected = false;
               }
            }
         }
      }

      #endregion
   }
}
