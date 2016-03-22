#include "qSegmentationWidget.h"
#include "vtkCuda2DTransferFunction.h"
#include "vtkCudaVoxelClassifier.h"
#include "vtkImageAppendComponents.h"
#include "vtkImageData.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkMetaImageWriter.h"
#include "vtkSystemIncludes.h"
#include "vtksys/SystemTools.hxx"
#include <QFileDialog>
#include <QVBoxLayout>

// ---------------------------------------------------------------------------------------
// Construction and destruction code
qSegmentationWidget::qSegmentationWidget( qTransferFunctionWindowWidget* p ) :
  QWidget(p)
{
  parent = p;
  window = 0;
  renderer = 0;
  mapper = 0;

  segmentationMenu = 0;
  setupMenu();
}

qSegmentationWidget::~qSegmentationWidget( )
{

}

void qSegmentationWidget::setupMenu()
{
  segmentationMenu = new QMenu("Segmentation",this);
  segmentNowOption = new QAction("Segment Now",this);
  segmentNowOption->setEnabled(true);
  segmentationMenu->addAction(segmentNowOption);

  connect(segmentNowOption,SIGNAL(triggered()),this,SLOT(segment()));
}

QMenu* qSegmentationWidget::getMenuOptions()
{
  return segmentationMenu;
}

void qSegmentationWidget::setStandardWidgets( vtkRenderWindow* w, vtkRenderer* r, vtkCudaDualImageVolumeMapper* c )
{
  window = w;
  renderer = r;
  mapper = c;
}
// ---------------------------------------------------------------------------------------
// Code to interface with the slots and user

void qSegmentationWidget::segment()
{
  vtkCudaVoxelClassifier* classifier = vtkCudaVoxelClassifier::New();
#if ( VTK_MAJOR_VERSION < 6 )
  classifier->SetInput(mapper->GetInput( mapper->GetCurrentFrame() ));
#else
  classifier->SetInputData(mapper->GetInput( mapper->GetCurrentFrame() ));
#endif
  classifier->SetClippingPlanes( mapper->GetClippingPlanes() );
  classifier->SetKeyholePlanes( mapper->GetKeyholePlanes() );
  classifier->SetFunction( mapper->GetFunction() );
  classifier->SetKeyholeFunction( mapper->GetKeyholeFunction() );
  classifier->Update();

  QString filename = QFileDialog::getSaveFileName(this, tr("Open File"), QDir::currentPath(),"Meta Image Files (*.mhd)" );

  if( filename.size() != 0 )
  {
    std::string rawfilename = vtksys::SystemTools::GetFilenameWithoutExtension( filename.toStdString() );
    rawfilename.append( ".raw" );
    vtkMetaImageWriter* writer = vtkMetaImageWriter::New();
    writer->SetCompression(false);
    writer->SetFileName( filename.toStdString().c_str() );
    writer->SetRAWFileName( rawfilename.c_str() );
#if ( VTK_MAJOR_VERSION < 6 )
    writer->SetInput( classifier->GetOutput() );
#else
    writer->SetInputConnection(classifier->GetOutputPort());
    writer->Update();
#endif
    writer->Write();
    writer->Delete();
  }
  classifier->Delete();
}