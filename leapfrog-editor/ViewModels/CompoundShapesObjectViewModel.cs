using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;


/*
 * The CompoundShapesObjectViewModel encompass the CompoundObject (incl its states, and behaviour) 
 * as well as all the ChildObjects of all states
 */


namespace LeapfrogEditor
{   // used to be: ConditionalSelectTreeViewViewModel
   public class CompoundShapesObjectViewModel : TreeViewViewModel
   {
      #region Declarations

      private CompoundShapesObject _modelBaseObject;


      private ShapeCollectionViewModel _shapes;

      #endregion

      #region Constructors

      public CompoundShapesObjectViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         CompoundShapesObject modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         _shapes = new ShapeCollectionViewModel(this, null, mainVm, enabled);
         ModelBaseObject = modelObject;
      }

      #endregion

      #region Properties

      public CompoundShapesObject ModelBaseObject
      {
         get { return _modelBaseObject; }
         set
         {
            _modelBaseObject = value;
            OnPropertyChanged("");
         }
      }

      public virtual string Name
         // For a FileCOViewModel, this property supplies the file name and it can not be set.
         // The ChildCOViewModel overrides this Property to return the 
         // Child name  (and to be able to set it)
      {
         get
         {
            if (this is FileCOViewModel)
            {
               FileCOViewModel fcvm = this as FileCOViewModel;

               string fileName = System.IO.Path.GetFileName(fcvm.FileName);

               return fileName;
            }

            return "Error: could not resolve name";
         }
         set
         {
            // Name of top level Compound Object can not be set
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

     virtual public Rect BoundingBox
      {
         get
         {
            BoundingBoxRect bbr = new BoundingBoxRect();

            if (ShapeCollection.Shapes.Count > 0)
            {
               foreach (object o in ShapeCollection.Shapes)
               {
                  if (o is LfShapeViewModel)
                  {
                     LfShapeViewModel shape = (LfShapeViewModel)o;

                     Rect cb = shape.BoundingBox;
                     cb.Offset(new Vector(shape.PosX, shape.PosY));
                     bbr.AddRect(cb);
                  }
               }
            }

            if (bbr.BoundingBox.IsEmpty)
            {
               return new Rect(0, 0, 100, 100);
            }

            return bbr.BoundingBox;
         }
      }

      #endregion

      #region Private Methods


      #endregion

      #region Public Methods

      // Override me
      virtual public LfShapeViewModel AddShape(LeftClickState shapeType, Point position)
      {
         return null;
      }

      // Override me
      virtual public void RemoveShape(LfShapeViewModel svm)
      {
      }


      virtual public void DeselectAllChildren()
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







      // This method returns with the supplied point in the scene's coordinate system
      // (Equal to the top level CompoundObject's coordinate system). 
      // The point is converted to the parent's coordinate system and the method is then
      // recursively called for the parent until the parent is null. In this case we are 
      // at the top level CompoundObject which is the scene. Then we returns the point
      public Point GetScenePointFromCoPoint(Point coPoint)
      {
         if ((ParentVm != null) && (ParentVm is CompoundObjectViewModel))
         {
            CompoundObjectViewModel covm = ParentVm as CompoundObjectViewModel;
            Point parentPoint = covm.ParentCoPoint(coPoint);
            return GetScenePointFromCoPoint(parentPoint);
         }
         else
         {
            return coPoint;
         }
      }

      public virtual Point GetCoPointFromScenePoint(Point scenePoint)
      {
         if (this is ChildCOViewModel)
         {
            ChildCOViewModel vm = this as ChildCOViewModel;

            Point parentPoint = vm.CoPointFromParent(scenePoint);
            return GetCoPointFromScenePoint(parentPoint);
         }
         else
         {
            return scenePoint;
         }
      }

      // Override me
      virtual public void GenerateTriangles()
      {
         foreach (object o in ShapeCollection.Shapes)
         {
            if (o is LfPolygonViewModel)
            {
               LfPolygonViewModel pvm = (LfPolygonViewModel)o;

               pvm.GenerateTriangles();
            }
         }
      }


      #endregion

   }
}
