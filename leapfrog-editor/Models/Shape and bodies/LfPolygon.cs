using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   [Serializable]
   public class LfPolygon : LfShape
    {
      #region Declarations

      private ObservableCollection<LfDragablePoint> _points = new ObservableCollection<LfDragablePoint>();
      private ObservableCollection<Triangle> _triangles = new ObservableCollection<Triangle>();

      #endregion

      #region Constructors

      public LfPolygon()
      {
      }

      #endregion

      #region Properties

      [XmlArray("vertices"), XmlArrayItem("vertex")]
      public ObservableCollection<LfDragablePoint> Points
      {
         get { return _points; }
         set { _points = value; }
      }

      [XmlArray("triangles"), XmlArrayItem("triangle")]
      public ObservableCollection<Triangle> Triangles
      {
         get { return _triangles; }
         set { _triangles = value; }
      }

      #endregion

      #region public Methods

      public void AddPoint(LfDragablePoint point)
      {
         Points.Add(point);
      }

      public void RemovePoint(LfDragablePoint point)
      {
         Points.Remove(point);
      }

      public void InsertPoint(LfDragablePoint insertMe, LfDragablePoint insertBeforeMe)
      {
         int index = 0;

         if (insertBeforeMe != null)
         {
            index = Points.IndexOf(insertBeforeMe);
         }

         if (index >= 0)
         {
            Points.Insert(index, insertMe);
         }
      }

      public void GenerateTriangles()
      {

      }

      #endregion
   }
}
