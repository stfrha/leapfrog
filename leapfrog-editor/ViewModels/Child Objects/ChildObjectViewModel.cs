using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;


/*
 * The ChildObjectViewModel encompass the CompoundObject (incl its states, and behaviour) 
 * as well as all the ChildObjects of all states
 */


namespace LeapfrogEditor
{
   // Used to be ConditionalSelectTreeViewViewModel
   public class ChildObjectViewModel : TreeViewViewModel
   {
      #region Declarations

      private ChildObject _modelObject;
      private ObservableCollection<ChildCOViewModel> _stateProperties = new ObservableCollection<ChildCOViewModel>();
      
      #endregion

      #region Constructors

      public ChildObjectViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm, 
         ChildObject modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;

         foreach (TStateProperties<ChildObjectStateProperties> cosp in ModelObject.StateProperties)
         {
            ChildCOViewModel cospvm = new ChildCOViewModel(this, parentVm, mainVm, cosp, enabled);
            StateProperties.Add(cospvm);

            // Since this COVM is the child of another COVM we need to check if the COVM
            // is a reference to a file or not. If it is, the children of the COVM shall 
            // not be selectable, i.e. they shall be disabled. If it it is not a file ref
            // it shall be selectable.
            if (cospvm.IsFileReferenceChild)
            {
               cospvm.BuildViewModel(false);
            }
            else
            {
               cospvm.BuildViewModel(true);
            }
         }
      }

      #endregion

      #region Properties

      public ChildObject ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
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
            OnPropertyChanged("RefName");
            if (TreeParent != null)
            {
               TreeParent.OnPropertyChanged("");
            }

            foreach (ChildCOViewModel covm in StateProperties)
            {
               covm.OnPropertyChanged("Name");
               covm.OnPropertyChanged("DispName");
            }
         }
      }

      public ObservableCollection<ChildCOViewModel> StateProperties
      {
         get { return _stateProperties; }
         set { _stateProperties = value; }
      }

      #endregion

      #region Private Methods


      #endregion

      #region Public Methods


      public void DeselectAllChildren()
      {
         foreach (ChildCOViewModel spvm in StateProperties)
         {
            spvm.IsSelected = false;
            spvm.DeselectAllChildren();
         }
      }

      #endregion

   }
}
