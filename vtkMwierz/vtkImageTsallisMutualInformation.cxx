/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageTsallisMutualInformation.cxx,v $
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
#include "vtkImageTsallisMutualInformation.h"

#if (VTK_MAJOR_VERSION >= 6)
#include <vtkExecutive.h>
#endif

#if (VTK_MAJOR_VERSION <= 5)
vtkCxxRevisionMacro(vtkImageTsallisMutualInformation, "$Revision: 1.1 $");
#endif
vtkStandardNewMacro(vtkImageTsallisMutualInformation);

//--------------------------------------------------------------------------
// The 'floor' function on x86 and mips is many times slower than these
// and is used a lot in this code, optimize for different CPU architectures
inline int vtkResliceFloor(double x)
{
#if defined mips || defined sparc
  return (int)((unsigned int)(x + 2147483648.0) - 2147483648U);
#elif defined i386 || defined _M_IX86
  unsigned int hilo[2];
  *((double *)hilo) = x + 103079215104.0;  // (2**(52-16))*1.5
  return (int)((hilo[1]<<16)|(hilo[0]>>16));
#else
  return int(floor(x));
#endif
}

inline int vtkResliceRound(double x)
{
  return vtkResliceFloor(x + 0.5);
}

inline int vtkResliceFloor(float x)
{
  return vtkResliceFloor((double)x);
}

inline int vtkResliceRound(float x)
{
  return vtkResliceRound((double)x);
}

// convert a float into an integer plus a fraction
inline int vtkResliceFloor(double x, double &f)
{
  int ix = vtkResliceFloor(x);
  f = x - ix;
  if (f < 0.0) f = 0.0;
  if (f > 1.0) f = 1.0;
  return ix;
}

//----------------------------------------------------------------------------
vtkImageTsallisMutualInformation::vtkImageTsallisMutualInformation()
{
  this->BinNumber[0] = 4096;
  this->BinNumber[1] = 4096;
  this->BinWidth[0] = 1.0;
  this->BinWidth[1] = 1.0;
  this->MaxIntensities[0] = 4095;
  this->MaxIntensities[1] = 4095;
  this->qValue = 1.5;
  this->ReverseStencil = 0;

  this->Threader = vtkMultiThreader::New();
  this->NumberOfThreads = this->Threader->GetNumberOfThreads();
}

//----------------------------------------------------------------------------
vtkImageTsallisMutualInformation::~vtkImageTsallisMutualInformation()
{
  this->Threader->Delete();
}

//----------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
void vtkImageTsallisMutualInformation::SetInput1(vtkImageData *input)
{
  this->vtkImageMultipleInputFilter::SetNthInput(0,input);
}
#else
void vtkImageTsallisMutualInformation::SetInput1Data(vtkImageData *input)
{
  this->vtkImageAlgorithm::SetInputData(0,input);
}
#endif

//----------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
void vtkImageTsallisMutualInformation::SetInput2(vtkImageData *input)
{
  this->vtkImageMultipleInputFilter::SetNthInput(1,input);
}
#else
void vtkImageTsallisMutualInformation::SetInput2Data(vtkImageData *input)
{
  this->vtkImageAlgorithm::SetInputData(1,input);
}
#endif

//----------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
void vtkImageTsallisMutualInformation::SetStencil(vtkImageStencilData *stencil)
{
  this->vtkProcessObject::SetNthInput(2, stencil);
}
#else
void vtkImageTsallisMutualInformation::SetStencilData(vtkImageStencilData *stencil)
{
  this->vtkImageAlgorithm::SetInputData(2, stencil);
}
#endif

//----------------------------------------------------------------------------
vtkImageData *vtkImageTsallisMutualInformation::GetInput1()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }

  return (vtkImageData *)(this->Inputs[0]);
}

//----------------------------------------------------------------------------
vtkImageData *vtkImageTsallisMutualInformation::GetInput2()
{
  if (this->NumberOfInputs < 2)
    {
    return NULL;
    }

  return (vtkImageData *)(this->Inputs[1]);
}

//----------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
vtkImageStencilData *vtkImageTsallisMutualInformation::GetStencil()
{
  if (this->NumberOfInputs < 3)
    {
    return NULL;
    }
  else
    {
    return (vtkImageStencilData *)(this->Inputs[2]);
    }
}
#else
vtkImageStencilData *vtkImageTsallisMutualInformation::GetStencil()
{
  if (this->GetNumberOfInputConnections(2) < 3)
    {
    return NULL;
    }
  else
    {
    return vtkImageStencilData::SafeDownCast(
      this->GetExecutive()->GetInputData(2, 0));
    }
}
#endif

//----------------------------------------------------------------------------
void vtkImageTsallisMutualInformation::SetBinNumber(int numS, int numT)
{
  this->BinNumber[0] = numS;
  this->BinNumber[1] = numT;
  this->BinWidth[0] = (double)this->MaxIntensities[0] / ((double)this->BinNumber[0] - 1.0);
  this->BinWidth[1] = (double)this->MaxIntensities[1] / ((double)this->BinNumber[1] - 1.0);
}

//----------------------------------------------------------------------------
void vtkImageTsallisMutualInformation::SetMaxIntensities(int maxS, int maxT)
{
  this->MaxIntensities[0] = maxS;
  this->MaxIntensities[1] = maxT;
  this->BinWidth[0] = (double)this->MaxIntensities[0] / ((double)this->BinNumber[0] - 1.0);
  this->BinWidth[1] = (double)this->MaxIntensities[1] / ((double)this->BinNumber[1] - 1.0);
}

//----------------------------------------------------------------------------
// Need to add histograms from different threads to create joint
// histogram image.
vtkImageData *vtkImageTsallisMutualInformation::GetOutput()
{
  if (this->NumberOfOutputs < 1)
    {
    return NULL;
    }

  int i, j, n = GetNumberOfThreads();
  double temp = 0.0;
  vtkImageData *HistST = (vtkImageData *)(this->Outputs[0]);

  for (i = 0; i < this->BinNumber[0]; i++)
    {
      for (j = 0; j < this->BinNumber[1]; j++)
  {
    temp = 0;
    for (int id = 0; id < n; id++)
      {
        temp += (double)this->ThreadHistST[id][i][j];
      }
    HistST->SetScalarComponentFromFloat(i,j,0,0,temp);
  }
    }

  return (vtkImageData *)(this->Outputs[0]);
}

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
// Handles the two input operations
template <class T>
void vtkImageTsallisMutualInformationExecute(vtkImageTsallisMutualInformation *self,
            vtkImageData *in1Data, T *in1Ptr,
            vtkImageData *in2Data, T *in2Ptr,
            int inExt[6], int id)
{
  int i,j,a,b;
  int idX, idY, idZ;
  int incX, incY, incZ;
  int maxX, maxY, maxZ;
  int pminX, pmaxX, iter;
  T *temp1Ptr, *temp2Ptr;
  vtkImageStencilData *stencil = self->GetStencil();

  for (i = 0; i < self->BinNumber[0]; i++)
    {
      self->ThreadHistS[id][i] = 0;
      self->ThreadHistT[id][i] = 0;
      for (j = 0; j < self->BinNumber[1]; j++)
   {
     self->ThreadHistST[id][i][j] = 0;
   }
    }

  // Find the region to loop over
  maxX = inExt[1] - inExt[0];
  maxY = inExt[3] - inExt[2];
  maxZ = inExt[5] - inExt[4];

  // Get increments to march through data
  in1Data->GetIncrements(incX, incY, incZ);

  // Loop over data within stencil sub-extents
  for (idZ = 0; idZ <= maxZ; idZ++)
    {
     for (idY = 0; idY <= maxY; idY++)
       {
   // Flag that we want the complementary extents
   iter = 0; if (self->GetReverseStencil()) iter = -1;

   pminX = 0; pmaxX = maxX;
   while ((stencil !=0 &&
     stencil->GetNextExtent(pminX, pmaxX, 0, maxX, idY, idZ+inExt[4], iter)) ||
    (stencil == 0 && iter++ == 0))
     {
       // Set up pointers to the sub-extents
       temp1Ptr = in1Ptr + (incZ * idZ + incY * idY + pminX);
       temp2Ptr = in2Ptr + (incZ * idZ + incY * idY + pminX);
       // Compute over the sub-extent
       for (idX = pminX; idX <= pmaxX; idX++)
         {
     a = vtkResliceRound(*temp1Ptr/double(self->BinWidth[0]));
     b = vtkResliceRound(*temp2Ptr/double(self->BinWidth[1]));
     if ((a < 0) | (a > 4095))
       {
         cout << "ERROR: Input 0 contains values < 0 or > 4095\n";
         exit(0);
       }
     if ((b < 0) | (b > 4095))
       {
         cout << "ERROR: Input 1 contains values < 0 or > 4095\n";
         exit(0);
       }
     self->ThreadHistS[id][a]++;
     self->ThreadHistT[id][b]++;
     self->ThreadHistST[id][a][b]++;
     temp1Ptr++;
     temp2Ptr++;
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
void vtkImageTsallisMutualInformation::ThreadedExecute1(vtkImageData **inData,
                 vtkImageData *vtkNotUsed(outData),
                 int inExt[6], int id)
{
  void *inPtr1;
  void *inPtr2;
  int ext0[6], ext1[6];

  if (inData[0] == NULL)
    {
    vtkErrorMacro(<< "Input 0 must be specified.");
    return;
    }

  if (inData[1] == NULL)
    {
      vtkErrorMacro(<< "Input 1 must be specified.");
      return;
    }

  inData[0]->GetWholeExtent(ext0);
  inData[1]->GetWholeExtent(ext1);

  if ((ext0[0] - ext1[0]) | (ext0[1] - ext1[1]) | (ext0[2] - ext1[2]) |
      (ext0[3] - ext1[3]) | (ext0[4] - ext1[4]) | (ext0[5] - ext1[5]))
    {
    vtkErrorMacro(<<"Inputs 0 and 1 must have the same extents.");
    return;
    }

  if ((inData[0]->GetNumberOfScalarComponents() > 1) |
      (inData[1]->GetNumberOfScalarComponents() > 1))
    {
      vtkErrorMacro("Inputs 0 and 1 must have 1 component each.");
      return;
    }

  if (inData[0]->GetScalarType() != inData[1]->GetScalarType())
    {
      vtkErrorMacro(<< "Inputs 0 and 1 must be of the same type");
      return;
    }

  // GetScalarPointer() for inData doesn't give the right results.
  inPtr1 = inData[0]->GetScalarPointerForExtent(inExt);
  inPtr2 = inData[1]->GetScalarPointerForExtent(inExt);

  switch (inData[0]->GetScalarType())
    {
#if (VTK_MAJOR_VERSION <= 5)
      vtkTemplateMacro7(vtkImageTsallisMutualInformationExecute,this,
      inData[0], (VTK_TT *)(inPtr1),
      inData[1], (VTK_TT *)(inPtr2),
      inExt, id);
#else
      vtkTemplateMacro(vtkImageTsallisMutualInformationExecute(this,
      inData[0], (VTK_TT *)(inPtr1),
      inData[1], (VTK_TT *)(inPtr2),
      inExt, id));
#endif
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}

//----------------------------------------------------------------------------
void vtkImageTsallisMutualInformation::ThreadedExecute2(int extS[6], int extST[6], int id)
{
  int i, j, n, N = GetNumberOfThreads();
  double temp1 = 0.0, temp2 = 0.0;

  this->ThreadEntropyS1[id] = 0.0;
  this->ThreadEntropyT1[id] = 0.0;
  this->ThreadEntropyST1[id] = 0.0;
  this->ThreadEntropyS2[id] = 0.0;
  this->ThreadEntropyT2[id] = 0.0;
  this->ThreadEntropyST2[id] = 0.0;
  this->ThreadCount[id] = 0.0;

  // Loop over S image histogram.
  for (i = extS[0]; i <= extS[1]; i++)
    {
      temp1 = 0.0;
      for (n = 0; n < N; n++)
  {
    temp1 += (double)this->ThreadHistS[n][i];
  }
      this->ThreadEntropyS1[id] += pow(temp1,this->qValue);
      this->ThreadEntropyS2[id] += temp1;
    }

  // Loop over T and ST histograms.
  for (j = extST[2]; j <= extST[3]; j++)
    {
      temp2 = 0.0;
      for (n = 0; n < N; n++)
  {
    temp2 += (double)this->ThreadHistT[n][j];
  }
      this->ThreadEntropyT1[id] += pow(temp2,this->qValue);
      this->ThreadEntropyT2[id] += temp2;
      for (i = extST[0]; i <= extST[1]; i++)
  {
     temp1 = 0.0;
     for (n = 0; n < N; n++)
       {
         temp1 += (double)this->ThreadHistST[n][i][j];
         this->ThreadCount[id] += (double)this->ThreadHistST[n][i][j];
       }
    this->ThreadEntropyST1[id] += pow(temp1,this->qValue);
    this->ThreadEntropyST2[id] += temp1;
  }
    }
}

//----------------------------------------------------------------------------
double vtkImageTsallisMutualInformation::GetResult()
{
  int id, n = GetNumberOfThreads();
  double entropyS1 = 0, entropyT1 = 0, entropyST1 = 0, result = 0;
  double entropyS2 = 0, entropyT2 = 0, entropyST2 = 0;
  double entropyS = 0, entropyT = 0, entropyST = 0;
  double count = 0.0;

  for (id = 0; id < n; id++)
    {
      entropyS1 += this->ThreadEntropyS1[id];
      entropyS2 += this->ThreadEntropyS2[id];
      entropyT1 += this->ThreadEntropyT1[id];
      entropyT2 += this->ThreadEntropyT2[id];
      entropyST1 += this->ThreadEntropyST1[id];
      entropyST2 += this->ThreadEntropyST2[id];
      count += this->ThreadCount[id];
    }

  if (count == 0) vtkErrorMacro( << "GetResult: No data to work with.");

  if (this->qValue != 1.0)
    {
      entropyS = ( 1.0 / (1.0 - this->qValue) *
       (entropyS1 / pow(count,this->qValue) - entropyS2 / count) );
      entropyT = ( 1.0 / (1.0 - this->qValue) *
       (entropyT1 / pow(count,this->qValue) - entropyT2 / count) );
      entropyST = ( 1.0 / (1.0 - this->qValue) *
       (entropyST1 / pow(count,this->qValue) - entropyST2 / count) );
    }
  else
    {
      vtkErrorMacro( << "GetResult: qValue = 1.");
    }

//   if ( entropyST * ( 1 + (1-this->qValue) * entropyST ) == 0 )
//     {
//       result = 1.0;
//     }
//   else
//     {
  result = ( ( entropyS + entropyT + (1 - this->qValue) *
         entropyS * entropyT - entropyST ) );// /
     //     ( entropyT + (1 - this->qValue) * entropyS * entropyT ) ) ;
     //     ( entropyST * ( 1 + (1-this->qValue) * entropyST ) ) );
     //    }

  return result;
}

//----------------------------------------------------------------------------
// Get ALL of the input.
void vtkImageTsallisMutualInformation::ComputeInputUpdateExtent(int inExt[6],
                   int outExt[6],
                   int vtkNotUsed(whichInput))
{
  int *wholeExtent = this->GetInput()->GetWholeExtent();
  memcpy(inExt, wholeExtent, 6*sizeof(int));
}

//----------------------------------------------------------------------------
struct vtkImageMultiThreadStruct
{
  vtkImageTsallisMutualInformation *Filter;
  vtkImageData   **Inputs;
  vtkImageData   *Output;
};

//----------------------------------------------------------------------------
// this mess is really a simple function. All it does is call
// the ThreadedExecute method after setting the correct
// extent for this thread. Its just a pain to calculate
// the correct extent.
VTK_THREAD_RETURN_TYPE vtkImageTsallisMutualInformationMultiThreadedExecute1( void *arg )
{
  vtkImageMultiThreadStruct *str;
  int ext[6], splitExt[6], total;
  int threadId, threadCount;

  threadId = ((ThreadInfoStruct *)(arg))->ThreadID;
  threadCount = ((ThreadInfoStruct *)(arg))->NumberOfThreads;

  str = (vtkImageMultiThreadStruct *)(((ThreadInfoStruct *)(arg))->UserData);

  // Thread over input images
  memcpy(ext,str->Filter->GetInput()->GetUpdateExtent(),
         sizeof(int)*6);

  // execute the actual method with appropriate extent
  // first find out how many pieces extent can be split into.
  total = str->Filter->SplitExtent(splitExt, ext, threadId, threadCount);

  if (threadId < total)
    {
    str->Filter->ThreadedExecute1(str->Inputs, str->Output, splitExt, threadId);
    }
  // else
  //   {
  //   otherwise don't use this thread. Sometimes the threads dont
  //   break up very well and it is just as efficient to leave a
  //   few threads idle.
  //   }

  return VTK_THREAD_RETURN_VALUE;
}

//----------------------------------------------------------------------------
// this mess is really a simple function. All it does is call
// the ThreadedExecute method after setting the correct
// extent for this thread. Its just a pain to calculate
// the correct extent.
VTK_THREAD_RETURN_TYPE vtkImageTsallisMutualInformationMultiThreadedExecute2( void *arg )
{
  vtkImageMultiThreadStruct *str;
  int ext[6], splitExtS[6], splitExtST[6], total;
  int threadId, threadCount;

  threadId = ((ThreadInfoStruct *)(arg))->ThreadID;
  threadCount = ((ThreadInfoStruct *)(arg))->NumberOfThreads;

  str = (vtkImageMultiThreadStruct *)(((ThreadInfoStruct *)(arg))->UserData);

  // Thread over S image histogram.
  ext[0] = 0; ext[1] = str->Filter->BinNumber[0] - 1;
  ext[2] = 0; ext[3] = 0;
  ext[4] = 0; ext[5] = 0;

  total = str->Filter->SplitExtent(splitExtS, ext, threadId, threadCount);

  // Thread over joint and T image histograms.
  ext[0] = 0; ext[1] = str->Filter->BinNumber[0] - 1;
  ext[2] = 0; ext[3] = str->Filter->BinNumber[1] - 1;
  ext[4] = 0; ext[5] = 0;

  // execute the actual method with appropriate extent
  // first find out how many pieces extent can be split into.
  total = str->Filter->SplitExtent(splitExtST, ext, threadId, threadCount);

  if (threadId < total)
    {
    str->Filter->ThreadedExecute2(splitExtS, splitExtST, threadId);
    }
  // else
  //   {
  //   otherwise don't use this thread. Sometimes the threads dont
  //   break up very well and it is just as efficient to leave a
  //   few threads idle.
  //   }

  return VTK_THREAD_RETURN_VALUE;
}


//----------------------------------------------------------------------------
void vtkImageTsallisMutualInformation::ExecuteData(vtkDataObject *out)
{
  vtkImageData *output = vtkImageData::SafeDownCast(out);
  if (!output)
    {
    vtkWarningMacro("ExecuteData called without ImageData output");
    return;
    }
  output->SetExtent(output->GetUpdateExtent());
  output->AllocateScalars();

  vtkImageMultiThreadStruct str;

  str.Filter = this;
  str.Inputs = (vtkImageData **)this->GetInputs();
  str.Output = output;

  this->Threader->SetNumberOfThreads(this->NumberOfThreads);

  // setup threading and the invoke threadedExecute
  this->Threader->SetSingleMethod(vtkImageTsallisMutualInformationMultiThreadedExecute1, &str);
  this->Threader->SingleMethodExecute();

  this->Threader->SetSingleMethod(vtkImageTsallisMutualInformationMultiThreadedExecute2, &str);
  this->Threader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
void vtkImageTsallisMutualInformation::ExecuteInformation(vtkImageData **inData,
                   vtkImageData *outData)
{
  // the two inputs are required to be of the same data type and extents.
  inData[0]->Update();
  inData[1]->Update();

  outData->SetWholeExtent(0, this->BinNumber[0]-1, 0, this->BinNumber[1]-1, 0, 0);
  outData->SetOrigin(0.0,0.0,0.0);
  outData->SetSpacing(this->BinWidth[0],this->BinWidth[1],1.0);
  outData->SetNumberOfScalarComponents(1);
  outData->SetScalarType(VTK_INT);

  // need to set the spacing and origin of the stencil to match the output
  vtkImageStencilData *stencil = this->GetStencil();
  if (stencil)
    {
    stencil->SetSpacing(inData[0]->GetSpacing());
    stencil->SetOrigin(inData[0]->GetOrigin());
    }
}

//----------------------------------------------------------------------------
void vtkImageTsallisMutualInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Stencil: " << this->GetStencil() << "\n";
  os << indent << "ReverseStencil: " << (this->ReverseStencil ? "On\n" : "Off\n");
  os << indent << "BinWidth: ( " << this->BinWidth[0] << ", " << this->BinWidth[1] << " )\n";
  os << indent << "BinNumber: ( "<<this->BinNumber[0] << ", " << this->BinNumber[1] << " )\n";
}
