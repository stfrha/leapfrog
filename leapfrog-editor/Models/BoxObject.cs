using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
   class BoxObject
   {
      #region Declarations

      private string _name;
      private string _texture;
      private double _posY;
      private double _posX;
      private double _width;
      private double _height;

      #endregion

      #region Constructors

      public BoxObject()
      {
         _name = "noName";
         _texture = "";
         _posX = 0;
         _posY = 0;
         _width = 0;
         _height = 0;
      }

      public BoxObject(string name, string texture, double posX, double posY, double width, double height)
      {
         _name = name;
         _texture = texture;
         _posX = posX;
         _posY = posY;
         _width = width;
         _height = height;
      }

      #endregion

      #region Properties

      public string Name
      {
         get { return _name; }
         set { _name = value; }
      }

      public string Texture
      {
         get { return _texture; }
         set { _texture = value; }
      }

      public double PosX
      {
         get { return _posX; }
         set { _posX = value; }
      }

      public double PosY
      {
         get { return _posY; }
         set { _posY = value; }
      }

      public double Width
      {
         get { return _width; }
         set { _width = value; }
      }

      public double Height
      {
         get { return _height; }
         set { _height = value; }
      }

      #endregion
   }
}
