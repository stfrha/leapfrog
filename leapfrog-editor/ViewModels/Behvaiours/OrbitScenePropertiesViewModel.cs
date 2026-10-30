using System;

namespace LeapfrogEditor
{
   // An orbit (re-entry) scene's planet (behaviour "orbitScene", see
   // OrbitSceneProperties)
   public class OrbitScenePropertiesViewModel : BehaviourViewModelBase
   {
      private OrbitSceneProperties ModelObject;

      public OrbitScenePropertiesViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         OrbitSceneProperties modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;
      }

      public OrbitSceneProperties LocalModelObject
      {
         get { return ModelObject; }
      }

      public string Texture
      {
         get { return LocalModelObject.Texture; }
         set
         {
            LocalModelObject.Texture = value;
            OnPropertyChanged("Texture");
         }
      }

      public string SpaceSceneBackground
      {
         get { return LocalModelObject.SpaceSceneBackground; }
         set
         {
            LocalModelObject.SpaceSceneBackground = value;
            OnPropertyChanged("SpaceSceneBackground");
         }
      }

      public string SpaceSceneFile
      {
         get { return LocalModelObject.SpaceSceneFile; }
         set
         {
            LocalModelObject.SpaceSceneFile = value;
            OnPropertyChanged("SpaceSceneFile");
         }
      }

      public string SurfaceFile
      {
         get { return LocalModelObject.SurfaceFile; }
         set
         {
            LocalModelObject.SurfaceFile = value;
            OnPropertyChanged("SurfaceFile");
         }
      }

      // Sites kept in the orbit scene itself (without a surface file)
      public int InlineSiteCount
      {
         get { return LocalModelObject.LandingSites.Count; }
      }
   }
}
