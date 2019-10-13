using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
   // This interface requires connection to MainVIewModel
   // to access it's mouse click and drag methods.

   interface IEditableShapeInterface
   {

      ShapeCollectionViewModel ShapeCollection
      {
         get;
         set;
      }

      Rect BoundingBox
      {
         get;
         set;
      }

      LfShapeViewModel AddShape(LeftClickState shapeType, Point position);

      void RemoveShape(LfShapeViewModel svm);

      LfDragablePointViewModel AddPoint(LfPolygonViewModel polyVm, Point position);

      LfShapeViewModel FindShape(string name, ShapeCollectionViewModel shapes);

      void DeselectAllChildren();

   }
}
