using System;
using System.Windows.Input;

namespace LeapfrogEditor
{
   // A landing site of a planet's surface (see LandingSite), in the
   // property editor's list and as a flag on the planet editor's ruler
   public class LandingSiteViewModel : MicroMvvm.ViewModelBase
   {
      private PlanetSurfacePropertiesViewModel _owner;
      private LandingSite _modelObject;

      public LandingSiteViewModel(PlanetSurfacePropertiesViewModel owner, LandingSite modelObject)
      {
         _owner = owner;
         _modelObject = modelObject;
      }

      public LandingSite ModelObject
      {
         get { return _modelObject; }
      }

      public string Name
      {
         get { return _modelObject.Name; }
         set
         {
            _modelObject.Name = value;
            OnPropertyChanged("Name");
            OnPropertyChanged("DisplayLabel");
         }
      }

      public string Label
      {
         get { return _modelObject.Label; }
         set
         {
            _modelObject.Label = value;
            OnPropertyChanged("Label");
            OnPropertyChanged("DisplayLabel");
         }
      }

      // What the game shows under the solution marker
      public string DisplayLabel
      {
         get { return string.IsNullOrEmpty(Label) ? Name : Label; }
      }

      // Degrees along the surface, growing to the right
      public double Angle
      {
         get { return _modelObject.Angle; }
         set
         {
            _modelObject.Angle = value;
            OnPropertyChanged("Angle");
            OnPropertyChanged("X");
            OnPropertyChanged("MarkerLeft");
         }
      }

      // Where the site is on the unrolled surface
      public double X
      {
         get { return PlanetSurfaceProperties.AngleToX(Angle); }
         set { Angle = Math.Round(PlanetSurfaceProperties.XToAngle(value), 2); }
      }

      // The game's site marker is 100 wide, centred on the site
      public double MarkerLeft
      {
         get { return X - 50.0; }
      }

      // "none" (the texture picker's choice for no texture) = no marker
      public string Texture
      {
         get { return _modelObject.Texture; }
         set
         {
            _modelObject.Texture = (value == "none") ? "" : value;
            OnPropertyChanged("Texture");
         }
      }

      public string Scene
      {
         get { return _modelObject.Scene; }
         set
         {
            _modelObject.Scene = value;
            OnPropertyChanged("Scene");
         }
      }

      public ICommand Remove
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => _owner.RemoveSite(this), parameter => true);
         }
      }
   }
}
