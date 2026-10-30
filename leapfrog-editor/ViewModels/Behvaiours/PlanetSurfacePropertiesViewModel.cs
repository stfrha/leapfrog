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
