using System;
using System.Collections.ObjectModel;
using System.Windows.Input;

namespace LeapfrogEditor
{
   // A planet's surface (behaviour "planetSurface", see
   // PlanetSurfaceProperties): the texture and the landing sites. While one
   // is edited, the canvas is the unrolled surface with a ruler in degrees
   // along its top (MainViewModel.PlanetView).
   public class PlanetSurfacePropertiesViewModel : BehaviourViewModelBase
   {
      private PlanetSurfaceProperties ModelObject;
      private ObservableCollection<LandingSiteViewModel> _sites = new ObservableCollection<LandingSiteViewModel>();

      public PlanetSurfacePropertiesViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         PlanetSurfaceProperties modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;

         foreach (LandingSite site in ModelObject.LandingSites)
         {
            _sites.Add(new LandingSiteViewModel(this, site));
         }
      }

      public PlanetSurfaceProperties LocalModelObject
      {
         get { return ModelObject; }
      }

      // "none" (the texture picker's choice) = the orbit scene's texture
      public string Texture
      {
         get { return LocalModelObject.Texture; }
         set
         {
            LocalModelObject.Texture = (value == "none") ? "" : value;
            OnPropertyChanged("Texture");
            OnPropertyChanged("GroundTile");
         }
      }

      public double TextureMeterWidth
      {
         get { return LocalModelObject.TextureMeterWidth; }
         set
         {
            LocalModelObject.TextureMeterWidth = Math.Max(0.0, value);
            OnPropertyChanged("TextureMeterWidth");
            OnPropertyChanged("GroundTile");
         }
      }

      public double TextureMeterHeight
      {
         get { return LocalModelObject.TextureMeterHeight; }
         set
         {
            LocalModelObject.TextureMeterHeight = Math.Max(0.0, value);
            OnPropertyChanged("TextureMeterHeight");
            OnPropertyChanged("GroundTile");
         }
      }

      // One tile of the ground texture on the canvas (ImageBrush viewport),
      // as the game sizes it (PlanetActor): the given size, or the
      // texture's pixels; a height of 0 keeps the proportions
      public System.Windows.Rect GroundTile
      {
         get
         {
            double pw = 512.0;
            double ph = 512.0;
            System.Windows.Media.Imaging.BitmapSource image =
               TextureImages.Get(Texture) as System.Windows.Media.Imaging.BitmapSource;
            if ((image != null) && (image.PixelWidth > 0) && (image.PixelHeight > 0))
            {
               pw = image.PixelWidth;
               ph = image.PixelHeight;
            }

            double w = (TextureMeterWidth > 0.0) ? TextureMeterWidth : pw;
            double h = (TextureMeterHeight > 0.0) ? TextureMeterHeight : w * ph / pw;
            return new System.Windows.Rect(0.0, 0.0, w, h);
         }
      }

      // In file order: the game numbers the sites so
      public ObservableCollection<LandingSiteViewModel> Sites
      {
         get { return _sites; }
      }

      // A new site, at the angle the ruler shows in the middle of the view
      public void AddSite(double angle)
      {
         int n = _sites.Count + 1;
         string name = "site" + n;
         while (HasSite(name))
         {
            n++;
            name = "site" + n;
         }

         LandingSite site = new LandingSite();
         site.Name = name;
         site.Angle = Math.Round(angle, 2);
         LocalModelObject.LandingSites.Add(site);
         _sites.Add(new LandingSiteViewModel(this, site));
      }

      public void RemoveSite(LandingSiteViewModel site)
      {
         LocalModelObject.LandingSites.Remove(site.ModelObject);
         _sites.Remove(site);
      }

      private bool HasSite(string name)
      {
         foreach (LandingSite s in LocalModelObject.LandingSites)
         {
            if (s.Name == name)
            {
               return true;
            }
         }
         return false;
      }

      public ICommand AddSiteCommand
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter =>
            {
               double angle = 0.0;
               MainWindow mw = System.Windows.Application.Current.MainWindow as MainWindow;
               if (mw != null)
               {
                  angle = mw.PlanetViewCentreAngle();
               }
               AddSite(angle);
            }, parameter => true);
         }
      }
   }
}
