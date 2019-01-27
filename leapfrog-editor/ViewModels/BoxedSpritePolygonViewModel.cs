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
   class BoxedSpritePolygonViewModel : EditablePolygonViewModel, IAnchorableInterface, IShapeInterface
   {
      #region Declarations

      private MainViewModel _mainVm;
      private BoxedSpritePolygon _modelObject;

      #endregion

      #region Constructors

      public BoxedSpritePolygonViewModel(MainViewModel mainVm, CompoundObjectViewModel parent, BoxedSpritePolygon modelObject) :
         base(mainVm, parent, modelObject)
      {
         MainVm = mainVm;
         ModelObject = modelObject;
         PolygonObject = (EditablePolygon)ModelObject;
      }

      #endregion

      #region Properties

      public MainViewModel MainVm
      {
         get { return _mainVm; }
         set { _mainVm = value; }
      }

      public BoxedSpritePolygon ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            PolygonObject = (EditablePolygon)_modelObject;
            OnPropertyChanged("");
         }
      }

      public string Name
      {
         get { return _modelObject.Name; }
         set
         {
            _modelObject.Name = value;
            OnPropertyChanged("Name");
         }
      }

      public string Texture
      {
         get { return _modelObject.Texture; }
         set
         {
            _modelObject.Texture = value;
            OnPropertyChanged("Texture");
         }
      }

      public double Angle
      {
         get { return _modelObject.Angle; }
         set
         {
            _modelObject.Angle = value;
            OnPropertyChanged("Angle");
         }
      }

      public double Width
      {
         get { return _modelObject.Width; }
         set
         {
            _modelObject.Width = value;
            OnPropertyChanged("Width");
         }
      }
      
      public double Height
      {
         get { return _modelObject.Height; }
         set
         {
            _modelObject.Height = value;
            OnPropertyChanged("Height");
         }
      }

      public double AnchorX
      {
         get { return _modelObject.AnchorX; }
         set
         {
            _modelObject.AnchorX = value;
            OnPropertyChanged("AnchorX");
         }
      }

      public double AnchorY
      {
         get { return _modelObject.AnchorY; }
         set
         {
            _modelObject.AnchorY = value;
            OnPropertyChanged("AnchorY");
         }
      }

      public string CollisionEntity
      {
         get { return _modelObject.CollisionEntity; }
         set
         {
            _modelObject.CollisionEntity = value;
            OnPropertyChanged("CollisionEntity");
         }
      }

      public uint ZLevel
      {
         get { return _modelObject.ZLevel; }
         set
         {
            _modelObject.ZLevel = value;
            OnPropertyChanged("ZLevel");
         }
      }

      public uint CollisionCategory
      {
         get { return _modelObject.CollisionCategory; }
         set
         {
            _modelObject.CollisionCategory = value;
            OnPropertyChanged("CollisionCategory");
         }
      }

      public uint CollisionMask
      {
         get { return _modelObject.CollisionMask; }
         set
         {
            _modelObject.CollisionMask = value;
            OnPropertyChanged("CollisionMask");
         }
      }

      #endregion

   }
}
