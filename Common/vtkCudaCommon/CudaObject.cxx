/*=========================================================================

  Program:   Visualization Toolkit
  Module:    CudaObject.cxx

  Copyright (c) John SH Baxter, Robarts Research Institute

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/** @file CudaObject.cxx
 *
 *  @brief An abstract class which uses CUDA
 *
 *  @author John Stuart Haberl Baxter (Dr. Peters' Lab (VASST) at Robarts Research Institute)
 *  @note First documented on June 12, 2012
 *
 *  @note Interacts primarily with the vtkCudaDeviceManager
 */

#include "CudaObject.h"
#include "vtkObjectFactory.h"
#include "cuda_runtime_api.h"

//----------------------------------------------------------------------------
void errorOut(CudaObject* self, const char* message)
{
  if (vtkObject::GetGlobalWarningDisplay())
  {
    vtkOStreamWrapper::EndlType endl;
    vtkOStreamWrapper::UseEndl(endl);
    vtkOStrStreamWrapper vtkmsg;
    vtkmsg << "ERROR: In " __FILE__ ", line " << __LINE__
           << "\n" << "CudaObject" << " (" << self
           << "): " << message << "\n\n";
    vtkmsg.rdbuf()->freeze(0);
    vtkObject::BreakOnError();
  }
}

//----------------------------------------------------------------------------
CudaObject::CudaObject(int d)
{
  //get the device managing utility
  this->DeviceManager = vtkCudaDeviceManager::Singleton();

  //get the starting device (default device 0)
  this->DeviceStream = 0;
  this->DeviceNumber = d;
  if( d < 0 || d >= this->DeviceManager->GetNumberOfDevices() )
  {
    this->DeviceNumber = 0;
  }
  bool errorThrown = this->DeviceManager->GetDevice(this, this->DeviceNumber);
  if(errorThrown)
  {
    errorOut(this,"Device selected cannot be retrieved.");
    this->DeviceNumber = -1;
    return;
  }

  //get a stream
  errorThrown = this->DeviceManager->GetStream(this, &(this->DeviceStream), this->DeviceNumber );
  if(errorThrown)
  {
    errorOut(this,"Device selected cannot be retrieved.");
    this->DeviceManager->ReturnDevice(this, this->DeviceNumber );
    this->DeviceNumber = -1;
    return;
  }
}

//----------------------------------------------------------------------------
CudaObject::~CudaObject()
{
  //synchronize remainder of stream and return control of the device
  this->CallSyncThreads();
  this->DeviceManager->ReturnDevice( this, this->DeviceNumber );
}

//----------------------------------------------------------------------------
void CudaObject::SetDevice( int d, int withData )
{
  int numberOfDevices = this->DeviceManager->GetNumberOfDevices();

  if( d < 0 || d >= numberOfDevices )
  {
    errorOut(this,"Device selected does not exist.");
    return;
  }

  if( this->DeviceNumber == -1 )
  {
    //set up a purely new device
    this->DeviceNumber = d;
    bool result = this->DeviceManager->GetDevice(this, this->DeviceNumber);
    if(result)
    {
      errorOut(this,"Device selected cannot be retrieved.");
      this->DeviceNumber = -1;
      return;
    }
    result = this->DeviceManager->GetStream(this, &(this->DeviceStream), this->DeviceNumber );
    if(result)
    {
      errorOut(this,"Device selected cannot be retrieved.");
      this->DeviceManager->ReturnDevice(this, this->DeviceNumber );
      this->DeviceNumber = -1;
      return;
    }
    this->Reinitialize(withData);
  }
  else if(this->DeviceNumber == d)
  {
    //if we are currently using that device, don't change anything
    return;
  }
  else
  {
    //finish all device business and set up a new device
    this->Deinitialize(withData);
    this->DeviceManager->ReturnStream(this, this->DeviceStream, this->DeviceNumber );
    this->DeviceStream = 0;
    this->DeviceManager->ReturnDevice(this, this->DeviceNumber );
    this->DeviceNumber = d;
    bool result = this->DeviceManager->GetDevice(this, this->DeviceNumber);
    if(result)
    {
      errorOut(this,"Device selected cannot be retrieved.");
      this->DeviceNumber = -1;
      return;
    }
    result = this->DeviceManager->GetStream(this, &(this->DeviceStream), this->DeviceNumber );
    if(result)
    {
      errorOut(this,"Device selected cannot be retrieved.");
      this->DeviceManager->ReturnDevice(this, this->DeviceNumber );
      this->DeviceNumber = -1;
      return;
    }
    this->Reinitialize(withData);
  }
}

//----------------------------------------------------------------------------
int CudaObject::GetDevice()
{
  return this->DeviceNumber;
}

//----------------------------------------------------------------------------
void CudaObject::ReserveGPU( )
{
  if( this->DeviceNumber == -1 )
  {
    errorOut(this,"No device set selected does not exist.");
    return;
  }
  if(this->DeviceManager->ReserveGPU(this->DeviceStream))
  {
    errorOut(this,"Error REserving GPU");
    return;
  }
}

//----------------------------------------------------------------------------
void CudaObject::CallSyncThreads( )
{
  if( this->DeviceNumber == -1 )
  {
    errorOut(this,"No device set selected does not exist.");
    return;
  }
  if(this->DeviceManager->SynchronizeStream(this->DeviceStream))
  {
    errorOut(this,"Error Synchronizing Streams");
    return;
  }
}

//----------------------------------------------------------------------------
cudaStream_t* CudaObject::GetStream( )
{
  return this->DeviceStream;
}

//----------------------------------------------------------------------------
void CudaObject::ReplicateObject( CudaObject* object, int withData )
{
  int oldDeviceNumber = this->DeviceNumber;
  this->SetDevice( object->DeviceNumber, withData );
  if(  this->DeviceStream != object->DeviceStream )
  {
    this->CallSyncThreads();
    this->DeviceManager->ReturnStream(this, this->DeviceStream, oldDeviceNumber);
    this->DeviceStream = 0;
    this->DeviceStream = object->DeviceStream;
    this->DeviceManager->GetStream( this, &(object->DeviceStream), object->DeviceNumber );
  }
}