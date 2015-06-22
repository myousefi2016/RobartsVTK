/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkCalcCentroid.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/04 14:34:34 $
  Version:   $Revision: 1.1 $
  Thanks:    Thanks to Yves who developed this class.

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkCalcCentroid.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkCalcCentroid* vtkCalcCentroid::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCalcCentroid");
  if(ret)
    {
    return (vtkCalcCentroid*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkCalcCentroid;
}


// Constructs with initial 0 values.
vtkCalcCentroid::vtkCalcCentroid()
{
  this->Input = NULL;
  this->Centroid[0] = 0.0;
  this->Centroid[1] = 0.0;
  this->Centroid[2] = 0.0;
  for (int i=0; i<=8; i++) 
    this->CovarianceMatrix[i]=0.0;
}

vtkCalcCentroid::~vtkCalcCentroid()
{
}

// Function to set up the covariance matrix
template <class T>
static int vtkCalculateCovarianceMatrix(vtkImageData * input, T *inPtr,
           double *centroid,
           int *inputExtent, double *covar)
{
  int inInc0, inInc1, inInc2;
  int idx0, idx1, idx2;
  T *inPtr0, *inPtr1, *inPtr2;

  double sxx = 0.0, sxy = 0.0, sxz = 0.0;
  double syx = 0.0, syy = 0.0, syz = 0.0;
  double szx = 0.0, szy = 0.0, szz = 0.0, si = 0.0;
  double dataCentroid[3];                   //centroid in data coordinates
  double *spacing = input->GetSpacing();
  double *origin = input->GetOrigin();

  dataCentroid[0] = (centroid[0] - origin[0])/spacing[0];
  dataCentroid[1] = (centroid[1] - origin[1])/spacing[1];
  dataCentroid[2] = (centroid[2] - origin[2])/spacing[2];
  
  input->GetIncrements(inInc0, inInc1, inInc2);

  inPtr2 = inPtr;
  for (idx2 = inputExtent[4]; idx2 <= inputExtent[5]; ++idx2)
    {
      inPtr1 = inPtr2;
      for (idx1 = inputExtent[2]; idx1 <= inputExtent[3]; ++idx1)
  {
    inPtr0 = inPtr1;
    for (idx0 = inputExtent[0]; idx0 <= inputExtent[1]; ++idx0)
      {
        sxx += (idx0-centroid[0]) * (idx0-centroid[0]) * *inPtr0;
        sxy += (idx0-centroid[0]) * (idx1-centroid[1]) * *inPtr0;
        sxz += (idx0-centroid[0]) * (idx2-centroid[2]) * *inPtr0;

        syx += (idx1-centroid[1]) * (idx0-centroid[0]) * *inPtr0;
        syy += (idx1-centroid[1]) * (idx1-centroid[1]) * *inPtr0;
        syz += (idx1-centroid[1]) * (idx2-centroid[2]) * *inPtr0;

        szx += (idx2-centroid[2]) * (idx0-centroid[0]) * *inPtr0;
        szy += (idx2-centroid[2]) * (idx1-centroid[1]) * *inPtr0;
        szz += (idx2-centroid[2]) * (idx2-centroid[2]) * *inPtr0;
        si += *inPtr0;

        inPtr0 += inInc0;
      }
    inPtr1 += inInc1;
  }
      inPtr2 += inInc2;
    }
  if (si != 0.0) {
    covar[0] = sxx/si; covar[1] = sxy/si; covar[2] = sxz/si;
    covar[3] = syx/si; covar[4] = syy/si; covar[5] = syz/si;
    covar[6] = szx/si; covar[7] = szy/si; covar[8] = szz/si;
    
    return(1);
    
  }
  else {
    return(0);
  }
}


// Description:
// This templated function executes the filter for any type of data.
template <class T>
static void vtkCalculateCentroid(vtkImageData *input, T *inPtr,
            int *inputExtent, double xyz[3])
{
  int inInc0, inInc1, inInc2;
  int idx0, idx1, idx2;
  T *inPtr0, *inPtr1, *inPtr2;
  
  double XMoment = 0.0;
  double YMoment = 0.0;
  double ZMoment = 0.0;
  double totalMass = 0.0;

  double *spacing = input->GetSpacing();
  double *origin = input->GetOrigin();
  
  input->GetIncrements(inInc0, inInc1, inInc2);

  inPtr2 = inPtr;
  for (idx2 = inputExtent[4]; idx2 <= inputExtent[5]; ++idx2)
    {
      inPtr1 = inPtr2;
      for (idx1 = inputExtent[2]; idx1 <= inputExtent[3]; ++idx1)
  {
    inPtr0 = inPtr1;
    for (idx0 = inputExtent[0]; idx0 <= inputExtent[1]; ++idx0)
      {
        XMoment += *inPtr0*idx0;
        YMoment += *inPtr0*idx1;
        ZMoment += *inPtr0*idx2;
        totalMass += *inPtr0;
        inPtr0 += inInc0;
      }
    inPtr1 += inInc1;
  }
      inPtr2 += inInc2;
    }
  
  xyz[0] = ((XMoment / totalMass) * spacing[0]) + origin[0];
  xyz[1] = ((YMoment / totalMass) * spacing[1]) + origin[1];
  xyz[2] = ((ZMoment / totalMass) * spacing[2]) + origin[2];
}


double *vtkCalcCentroid::GetCentroid ()
{
  this->ComputeCentroid();
  return this->Centroid;
}

double *vtkCalcCentroid::GetCovarianceMatrix()
{
  this->ComputeCovarianceMatrix();
  return this->CovarianceMatrix;
}



void vtkCalcCentroid::ComputeCentroid()
{
  double centroid[3];
  
  // make sure input is available
  if ( ! this->Input )
    {
    vtkErrorMacro(<< "No input...can't execute!");
    }

  int inputExtent[6];
  this->Input->GetWholeExtent(inputExtent);
  this->Input->SetUpdateExtent(inputExtent);
#if (VTK_MAJOR_VERSION <= 5)
  this->Input->Update();
#endif
  
  void *inPtr = this->Input->GetScalarPointerForExtent(inputExtent);
  
  switch (this->Input->GetScalarType())
      {
    case VTK_DOUBLE:
      vtkCalculateCentroid(this->Input, (double *)(inPtr), 
        inputExtent, centroid);
      break;
    case VTK_FLOAT:
      vtkCalculateCentroid(this->Input, (float *)(inPtr), 
        inputExtent, centroid);
      break;
    case VTK_LONG:
      vtkCalculateCentroid(this->Input, (long *)(inPtr), 
        inputExtent, centroid);
      break;
    case VTK_UNSIGNED_LONG:
      vtkCalculateCentroid(this->Input, (unsigned long *)(inPtr), 
        inputExtent, centroid);
      break;
    case VTK_INT:
      vtkCalculateCentroid(this->Input, (int *)(inPtr), 
       inputExtent, centroid);
      break;
    case VTK_UNSIGNED_INT:
      vtkCalculateCentroid(this->Input, (unsigned int *)(inPtr), 
        inputExtent, centroid);
      break;
    case VTK_SHORT:
      vtkCalculateCentroid(this->Input, (short *)(inPtr), 
        inputExtent, centroid);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkCalculateCentroid(this->Input, (unsigned short *)(inPtr), 
        inputExtent, centroid);
      break;
    case VTK_CHAR:
      vtkCalculateCentroid(this->Input, (char *)(inPtr), 
        inputExtent, centroid);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkCalculateCentroid(this->Input, (unsigned char *)(inPtr), 
        inputExtent, centroid);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
    }
  if ( this->Input->ShouldIReleaseData() )
    {
      this->Input->ReleaseData();
    }
  this->Centroid[0] = centroid[0];
  this->Centroid[1] = centroid[1];
  this->Centroid[2] = centroid[2];
}


void vtkCalcCentroid::ComputeCovarianceMatrix()
{

  
  // make sure input is available
  if ( ! this->Input )
    {
    vtkErrorMacro(<< "No input...can't execute!");
    }

  int inputExtent[6];
  this->Input->GetWholeExtent(inputExtent);
  this->Input->SetUpdateExtent(inputExtent);
#if (VTK_MAJOR_VERSION <= 5)
  this->Input->Update();
#endif
  
  void *inPtr = this->Input->GetScalarPointerForExtent(inputExtent);
  
  switch (this->Input->GetScalarType())
      {
    case VTK_DOUBLE:
      vtkCalculateCovarianceMatrix(this->Input, (double *)(inPtr),
           this->Centroid, inputExtent, this->CovarianceMatrix);
      break;
    case VTK_FLOAT:
      vtkCalculateCovarianceMatrix(this->Input, (float *)(inPtr),
           this->Centroid, inputExtent, this->CovarianceMatrix);
      break;
    case VTK_LONG:
      vtkCalculateCovarianceMatrix(this->Input, (long *)(inPtr),
           this->Centroid, inputExtent, this->CovarianceMatrix);
      break;
    case VTK_UNSIGNED_LONG:
      vtkCalculateCovarianceMatrix(this->Input, (unsigned long *)(inPtr),
           this->Centroid, inputExtent, this->CovarianceMatrix);
      break;
    case VTK_INT:
      vtkCalculateCovarianceMatrix(this->Input, (int *)(inPtr),
           this->Centroid, inputExtent, this->CovarianceMatrix);
      break;
    case VTK_UNSIGNED_INT:
      vtkCalculateCovarianceMatrix(this->Input, (unsigned int *)(inPtr),
           this->Centroid, inputExtent, this->CovarianceMatrix);
      break;
    case VTK_SHORT:
      vtkCalculateCovarianceMatrix(this->Input, (short *)(inPtr),
           this->Centroid, inputExtent, this->CovarianceMatrix);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkCalculateCovarianceMatrix(this->Input, (unsigned short *)(inPtr),
           this->Centroid, inputExtent, this->CovarianceMatrix);
      break;
    case VTK_CHAR:
      vtkCalculateCovarianceMatrix(this->Input, (char *)(inPtr),
           this->Centroid, inputExtent, this->CovarianceMatrix);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkCalculateCovarianceMatrix(this->Input, (unsigned char *)(inPtr),
           this->Centroid, inputExtent, this->CovarianceMatrix);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
    }



  if ( this->Input->ShouldIReleaseData() )
    {
      this->Input->ReleaseData();
    }
}





void vtkCalcCentroid::PrintSelf(ostream& os, vtkIndent indent)
{
  double *mat = this->CovarianceMatrix;
  vtkObject::PrintSelf(os,indent);

  if (!this->GetInput()) 
    {
    return;
    }
  os << indent << "Centroid: [" << this->Centroid[0] << "," <<
    this->Centroid[1] << "," << this->Centroid[2] << "]\n";
  os << indent << "Covariance Matrix:\n" \
     << indent << "[" << mat[0] << "," << mat[1] << "," << mat[2] << "]\n"\
     << indent << "[" << mat[3] << "," << mat[4] << "," << mat[5] << "]\n"\
     << indent << "[" << mat[6] << "," << mat[7] << "," << mat[8] << "]\n";
    
}
