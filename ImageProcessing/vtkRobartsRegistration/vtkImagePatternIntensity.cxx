/*=========================================================================

  Program:   Robarts Visualization Toolkit
  Module:    $RCSfile: vtkImagePatternIntensity.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/04 14:34:35 $
  Version:   $Revision: 1.1 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImagePatternIntensity.h"

#include <vtkExecutive.h>

vtkStandardNewMacro(vtkImagePatternIntensity);

//----------------------------------------------------------------------------
vtkImagePatternIntensity::vtkImagePatternIntensity()
{
  this->ReverseStencil = 0;
}

//----------------------------------------------------------------------------
void vtkImagePatternIntensity::SetInput1Data(vtkImageData *input)
{
  this->SetInputData(0,input);
}

//----------------------------------------------------------------------------
void vtkImagePatternIntensity::SetInput2Data(vtkImageData *input)
{
  this->SetInputData(1,input);
}

//----------------------------------------------------------------------------
void vtkImagePatternIntensity::SetStencilData(vtkImageStencilData *stencil)
{
  this->SetInputData(2, stencil);
}

//----------------------------------------------------------------------------
vtkImageData *vtkImagePatternIntensity::GetInput1()
{
  if (this->GetNumberOfInputConnections(0) < 1)
  {
    return NULL;
  }

  return vtkImageData::SafeDownCast( this->GetExecutive()->GetInputData(0, 0) );
}

//----------------------------------------------------------------------------
vtkImageData *vtkImagePatternIntensity::GetInput2()
{
  if (this->GetNumberOfInputConnections(1) < 1)
  {
    return NULL;
  }

  return vtkImageData::SafeDownCast( this->GetExecutive()->GetInputData(1, 0) );
}

//----------------------------------------------------------------------------
vtkImageStencilData *vtkImagePatternIntensity::GetStencil()
{
  if (this->GetNumberOfInputConnections(2) < 1)
  {
    return NULL;
  }

  return vtkImageStencilData::SafeDownCast( this->GetExecutive()->GetInputData(2, 0) );
}

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
// Handles the two input operations
template <class T>
void vtkImagePatternIntensityExecute(vtkImagePatternIntensity *self,
                                     vtkImageData *diffData, T *diffPtr,
                                     int outExt[6], int id)
{
  int totExt[6];
  int idX, idY, idZ;
  int id2X, id2Y, id2Z;
  vtkIdType incX, incY, incZ;
  int maxX, maxY, maxZ;
  int pminX, pmaxX, iter;
  int rX, rY, rZ;
  T *temp1Ptr, *temp2Ptr;
  vtkImageStencilData *stencil = self->GetStencil();

  self->ThreadPatternIntensity[id] = 0;

  diffData->GetExtent(totExt);

  // Find the region to loop over
  maxX = outExt[1] - outExt[0];
  maxY = outExt[3] - outExt[2];
  maxZ = outExt[5] - outExt[4];

  // Get increments to march through data
  diffData->GetIncrements(incX, incY, incZ);

  // Loop over data within stencil sub-extents
  for (idZ = 0; idZ <= maxZ; idZ++)
  {
    for (idY = 0; idY <= maxY; idY++)
    {
      // Flag that we want the complementary extents
      iter = 0;
      if (self->GetReverseStencil())
      {
        iter = -1;
      }

      pminX = 0;
      pmaxX = maxX;
      while ((stencil !=0 &&
              stencil->GetNextExtent(pminX, pmaxX, 0, maxX, idY, idZ+outExt[4], iter)) ||
             (stencil == 0 && iter++ == 0))
      {
        // Set up pointers to the sub-extents
        temp1Ptr = diffPtr + (incZ * idZ + incY * idY + pminX);
        // Compute over the sub-extent
        for (idX = pminX; idX <= pmaxX; idX++)
        {

          for (id2Z = -3; id2Z <= 3; id2Z++)
          {
            for (id2Y = -3; id2Y <= 3; id2Y++)
            {
              for (id2X = -3; id2X <= 3; id2X++)
              {
                if (sqrt((double)(id2X*id2X) + (double)(id2Y*id2Y) + (double)(id2Z*id2Z)) <= 3.0)
                {
                  rX = idX+id2X;
                  rY = idY+id2Y;
                  rZ = idZ+id2Z;
                  if ( (rX >= totExt[0]) && (rX <= totExt[1]) &&
                       (rY >= totExt[2]) && (rY <= totExt[3]) &&
                       (rZ >= totExt[4]) && (rZ <= totExt[5]) )
                  {
                    temp2Ptr = diffPtr + (incZ * rZ + incY * rY + rX);
                    self->ThreadPatternIntensity[id] += 100.0 / (100 + (*temp1Ptr - *temp2Ptr) * (*temp1Ptr - *temp2Ptr));
                  }
                  else
                  {
                    self->ThreadPatternIntensity[id] += 100.0 / (100 + (*temp1Ptr * *temp1Ptr));
                  }
                }
              }
            }
          }
          temp1Ptr++;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImagePatternIntensity::ThreadedExecute(vtkImageData **inData,
    vtkImageData *vtkNotUsed(outData),
    int outExt[6], int id)
{
  void *diffPtr;

  vtkDebugMacro( "Execute: inData = " << inData);

  if (inData[0] == NULL)
  {
    vtkErrorMacro( "Input " << 0 << " must be specified.");
    return;
  }

  if (inData[1] == NULL)
  {
    vtkErrorMacro( "Input " << 1 << " must be specified.");
    return;
  }

  if ((inData[0]->GetScalarType() != inData[1]->GetScalarType()))
  {
    vtkErrorMacro( "Execute: Inputs must be of the same ScalarType");
    return;
  }

  // this filter expects that inputs that have the same number of components
  if ((inData[0]->GetNumberOfScalarComponents() != inData[1]->GetNumberOfScalarComponents()))
  {
    vtkErrorMacro( "Execute: input1 NumberOfScalarComponents, "
                   << inData[0]->GetNumberOfScalarComponents()
                   << ", must match input2 NumberOfScalarComponents "
                   << inData[1]->GetNumberOfScalarComponents());
    return;
  }

  vtkImageMathematics *diffMath = vtkImageMathematics::New();
  diffMath->SetInput1Data(inData[0]);
  diffMath->SetInput2Data(inData[1]);
  diffMath->SetOperationToSubtract();
  diffMath->Update();

  diffPtr = diffMath->GetOutput()->GetScalarPointer();

  switch (diffMath->GetOutput()->GetScalarType())
  {
    vtkTemplateMacro(vtkImagePatternIntensityExecute(this,
                     diffMath->GetOutput(), (VTK_TT *)(diffPtr),
                     outExt, id));
  default:
    vtkErrorMacro( "Execute: Unknown ScalarType");
    return;
  }
}

//----------------------------------------------------------------------------
double vtkImagePatternIntensity::GetResult()
{
  int n = GetNumberOfThreads();
  double result = 0.0;

  for (int id = 0; id < n; id++)
  {
    result += this->ThreadPatternIntensity[id];
  }

  if (result < 0)
  {
    vtkErrorMacro( "GetResult: result < 0");
  }

  return result;
}

//----------------------------------------------------------------------------
void vtkImagePatternIntensity::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Input 1: "<< this->GetInput1() << "\n";
  os << indent << "Input 2: "<< this->GetInput2() << "\n";
  os << indent << "Stencil: " << this->GetStencil() << "\n";
  os << indent << "ReverseStencil: " << (this->ReverseStencil ? "On\n" : "Off\n");
}
