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
   class StaticPolygonViewModel : ScalableTexturePolygonViewModel
   {
      #region Declarations

      private StaticPolygon _modelObject;
      private ObservableCollection<DragablePointViewModel> _pointVms = new ObservableCollection<DragablePointViewModel>();

      #endregion

      #region Constructors

      public StaticPolygonViewModel(MainViewModel mainVm, CompoundObjectViewModel parent, StaticPolygon modelObject) :
         base(mainVm, parent, modelObject)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public StaticPolygon ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            PolygonObject = (EditablePolygon)_modelObject;
            TexturePolygonObject = (ScalableTexturePolygon)_modelObject;
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
