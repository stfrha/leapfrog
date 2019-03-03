using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace LeapfrogEditor
{
   public class SpawnObjectViewModel : TreeViewViewModel
   {
      #region Declarations

      private SpawnObject ModelObject;

      private ISpawnObjectParentVmInterface _spawnParent;

      // The _spawnChildObject field is a collection because it must be added to the itemssource
      private ObservableCollection<ChildObjectViewModel> _spawnChildObject = new ObservableCollection<ChildObjectViewModel>();

      #endregion

      #region Constructors

      public SpawnObjectViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         ISpawnObjectParentVmInterface spawnParent,
         SpawnObject modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm)
      {
         ModelObject = modelObject;
         SpawnParent = spawnParent;

         ChildObjectViewModel chvm = new ChildObjectViewModel(this, parentVm, mainVm, modelObject.MyChildObject);
         SpawnChildObject.Add(chvm);
      }

      #endregion

      #region Properties

      public SpawnObject LocalModelObject
      {
         get { return ModelObject; }
      }

      public ISpawnObjectParentVmInterface SpawnParent
      {
         get { return _spawnParent; }
         set
         {
            _spawnParent = value;
            OnPropertyChanged("SpawnParent");
         }
      }

      public double ProbabilityFactor
      {
         get { return LocalModelObject.ProbabilityFactor; }
         set
         {
            LocalModelObject.ProbabilityFactor = value;
            OnPropertyChanged("ProbabilityFactor");
            OnPropertyChanged("Headline");
         }
      }

      public ObservableCollection<ChildObjectViewModel> SpawnChildObject
      {
         get { return _spawnChildObject; }
         set { _spawnChildObject = value; }
      }

      public string Headline
      {
         get { return LocalModelObject.MyChildObject.Name + "(" + LocalModelObject.ProbabilityFactor.ToString() + ")";  }
      }

      //public ChildObject MyChildObject
      //{
      //   get { return LocalModelObject.MyChildObject; }
      //   set
      //   {
      //      LocalModelObject.MyChildObject = value;
      //      OnPropertyChanged("MyChildObject");
      //      OnPropertyChanged("Headline");
      //   }
      //}

      #endregion

      #region private Methods

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      #endregion
   }
}
