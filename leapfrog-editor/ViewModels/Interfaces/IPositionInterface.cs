﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
   // This interface requires connection to MainVIewModel
   // to access it's mouse click and drag methods.

   interface IPositionInterface : IMainVmInterface
   {
      double PosX
      {
         get;
         set;
      }

      double PosY
      {
         get;
         set;
      }

      double AbsPosX
      {
         get;
      }

      double AbsPosY
      {
         get;
      }
   }
}
