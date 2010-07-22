/***************************************************************************
Copyright (C) 2008 by the Tonatiuh Software Development Team.

This file is part of Tonatiuh.

Tonatiuh program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.


Acknowledgments:

The development of Tonatiuh was started on 2004 by Dr. Manuel J. Blanco,
then Chair of the Department of Engineering of the University of Texas at
Brownsville. From May 2004 to July 2008, it was supported by the Department
of Energy (DOE) and the National Renewable Energy Laboratory (NREL) under
the Minority Research Associate (MURA) Program Subcontract ACQ-4-33623-06.
During 2007, NREL also contributed to the validation of Tonatiuh under the
framework of the Memorandum of Understanding signed with the Spanish
National Renewable Energy Centre (CENER) on February, 20, 2007 (MOU#NREL-07-117).
Since June 2006, the development of Tonatiuh is being led by the CENER, under the
direction of Dr. Blanco, now Director of CENER Solar Thermal Energy Department.

Developers: Manuel J. Blanco (mblanco@cener.com), Amaia Mutuberria, Victor Martin.

Contributors: Javier Garcia-Barberena, I�aki Perez, Inigo Pagola,  Gilda Jimenez,
Juana Amieva, Azael Mancillas, Cesar Cantu.
***************************************************************************/

#include <iostream>

#include <QFutureWatcher>
#include <QMutex>
#include <QtConcurrentMap>

#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodekits/SoSceneKit.h>

#include "Document.h"
#include "SceneModel.h"
#include "ScriptRayTracer.h"
#include "RandomDeviate.h"
#include "RandomDeviateFactory.h"
#include "RayTracer.h"
#include "tgf.h"
#include "TLightKit.h"
#include "TPhotonMap.h"
#include "TPhotonMapFactory.h"
#include "trf.h"
#include "TSeparatorKit.h"
#include "TShape.h"
#include "TSunShape.h"


ScriptRayTracer::ScriptRayTracer( QVector< TPhotonMapFactory* > listTPhotonMapFactory, QVector< RandomDeviateFactory* > listRandomDeviateFactory )
:m_exportFileName( 0 ),
m_exportSurfaceName( 0 ),
m_exportSurfaceInGlobalCoordinates( true ),
m_modelFileName( 0 ),
m_numberOfRays( 0 ),
m_TPhotonMapFactoryList( listTPhotonMapFactory ),
m_photonMap( 0 ),
m_RandomDeviateFactoryList( listRandomDeviateFactory ),
m_randomDeviate( 0 ),
m_sunAzimuth( 0 ),
m_sunElevation( 0 ),
m_sunDistance( 0 )
{

}

ScriptRayTracer::~ScriptRayTracer()
{
	delete m_photonMap;
	delete m_randomDeviate;
}

void ScriptRayTracer::Clear()
{
	m_exportFileName.clear();
	m_exportSurfaceName.clear();
	m_exportSurfaceInGlobalCoordinates = true;
	m_modelFileName.clear();
	m_numberOfRays = 0;
	delete m_photonMap;
	m_photonMap = 0;
	delete m_randomDeviate;
	m_randomDeviate = 0;
	m_sunAzimuth = 0;
	m_sunElevation = 0;
	m_sunDistance = 0;
}

void ScriptRayTracer::SetExportFileName( QString filename )
{
	m_exportFileName = filename;
}


void ScriptRayTracer::SetExportSurfaceName( QString surfaceName )
{
	m_exportSurfaceName = surfaceName;
}


void ScriptRayTracer::SetExportSurfaceCoordinates( bool globalCoordinates )
{
	m_exportSurfaceInGlobalCoordinates = globalCoordinates;
}

void ScriptRayTracer::SetTonatiuhModelFile ( QString filename )
{
	m_modelFileName = filename;
}



void ScriptRayTracer::SetNumberOfRays( double nrays )
{
	m_numberOfRays = nrays;
}


void ScriptRayTracer::SetPhotonMapType( QString typeName )
{
	QVector< QString > photonMapNames;

	for( int i = 0; i < m_TPhotonMapFactoryList.size(); i++ )
		photonMapNames<< m_TPhotonMapFactoryList[i]->TPhotonMapName();

	int selectedPhotonMap = photonMapNames.indexOf( typeName );
	if( selectedPhotonMap > -1 )	m_photonMap = m_TPhotonMapFactoryList[selectedPhotonMap]->CreateTPhotonMap();
	else m_photonMap = 0;
}

void ScriptRayTracer::SetRandomDeviateType( QString typeName )
{
	QVector< QString > randomGeneratorsNames;

	for( int i = 0; i < m_RandomDeviateFactoryList.size(); i++ )
		randomGeneratorsNames<< m_RandomDeviateFactoryList[i]->RandomDeviateName();

	int selectedRandom = randomGeneratorsNames.indexOf( typeName );

	if( selectedRandom > -1 )	m_randomDeviate = m_RandomDeviateFactoryList[selectedRandom]->CreateRandomDeviate();
	else m_randomDeviate = 0;
}

/*!
 * Saves the sun position \a azimuth value. \a azimuth  is in degrees.
 */
void ScriptRayTracer::SetSunAzimtuh( double azimuth )
{
	m_sunAzimuth = azimuth * tgc::Degree;
}

/*!
 * Saves the sun position \a elevation value. \a elevation  is in degrees.
 */
void ScriptRayTracer::SetSunElevation( double elevation )
{
	m_sunElevation = elevation * tgc::Degree;
}

/*!
 * Saves the sun position \a distance value.
 */
void ScriptRayTracer::SetSunDistance( double distance )
{
	m_sunDistance = distance;
}

int ScriptRayTracer::Trace()
{
	std::cout<<"m_exportFileName: "<<m_exportFileName.toStdString()<<std::endl;
	std::cout<<"m_exportSurfaceName: "<<m_exportSurfaceName.toStdString()<<std::endl;
	std::cout<<"m_numberOfRays: "<<m_numberOfRays<<std::endl;
	std::cout<<"m_sunAzimuth: "<<m_sunAzimuth<<std::endl;
	std::cout<<"m_sunElevation: "<<m_sunElevation<<std::endl;
	std::cout<<"m_sunDistance: "<<m_sunDistance<<std::endl;

	if( m_modelFileName.isEmpty() ) return 0;
	std::cout<<"m_modelFileName: "<<m_modelFileName.toStdString()<<std::endl;

	if( !m_photonMap ) return 0;
	std::cout<<"m_photonMap "<<std::endl;
	if( !m_randomDeviate ) return 0;
	std::cout<<"m_randomDeviate "<<std::endl;

	Document document;
	if( !document.ReadFile( m_modelFileName ) )	return 0;

	SceneModel sceneModel( 0 );
	sceneModel.SetCoinRoot( *document.GetRoot() );

	SoSceneKit* coinScene = document.GetSceneKit();
	sceneModel.SetCoinScene( *coinScene );

	QModelIndex sceneIndex;
	InstanceNode* sceneInstance = sceneModel.NodeFromIndex( sceneIndex );
	if ( !sceneInstance )  return 0;
	std::cout<<"rootNode: "<<sceneInstance->GetNode()->getTypeId().getName().getString()<<std::endl;

	InstanceNode* rootSeparatorInstance = sceneInstance->children[1];
	if( !rootSeparatorInstance ) return 0;

	InstanceNode* lightInstance = sceneInstance->children[0];
	if ( !lightInstance ) return 0;


	if ( !coinScene->getPart( "lightList[0]", false ) )	return 0;
	TLightKit* lightKit = static_cast< TLightKit* >( coinScene->getPart( "lightList[0]", true ) );
	lightKit->ChangePosition( m_sunAzimuth, tgc::Pi/2 - m_sunElevation, m_sunDistance );

	if( !lightKit->getPart( "tsunshape", false ) ) return 0;
	TSunShape* sunShape = static_cast< TSunShape * >( lightKit->getPart( "tsunshape", false ) );
	double irradiance = sunShape->GetIrradiance();

	if( !lightKit->getPart( "icon", false ) ) return 0;
	TShape* raycastingSurface = static_cast< TShape * >( lightKit->getPart( "icon", false ) );
	double inputAperture = raycastingSurface->GetArea();

	if( !lightKit->getPart( "transform" ,true ) ) return 0;
	SoTransform* lightTransform = static_cast< SoTransform* >( lightKit->getPart( "transform" ,true ) );


	ComputeSceneTreeMap( rootSeparatorInstance, Transform( new Matrix4x4 ) );

	QVector< double > raysPerThread;
	const int maximumValueProgressScale = 100;
	unsigned long  t1 = m_numberOfRays / maximumValueProgressScale;
	for( int progressCount = 0; progressCount < maximumValueProgressScale; ++ progressCount )
		raysPerThread<< t1;

	if( ( t1 * maximumValueProgressScale ) < m_numberOfRays )	raysPerThread<< ( m_numberOfRays-( t1* maximumValueProgressScale) );

	Transform lightToWorld = tgf::TransformFromSoTransform( lightTransform );

	//ParallelRandomDeviate* m_pParallelRand = new ParallelRandomDeviate( *m_rand,140000 );
	// Create a QFutureWatcher and conncect signals and slots.
	QFutureWatcher< TPhotonMap* > futureWatcher;

	QMutex mutex;
	QFuture< TPhotonMap* > photonMap = QtConcurrent::mappedReduced( raysPerThread, RayTracer(  rootSeparatorInstance, raycastingSurface, sunShape, lightToWorld, *m_randomDeviate, &mutex, m_photonMap ), trf::CreatePhotonMap, QtConcurrent::UnorderedReduce );
	futureWatcher.setFuture( photonMap );

	futureWatcher.waitForFinished();

	if( !m_photonMap ) return 1;
	if( m_photonMap->StoredPhotons() == 0 )	return 1;
	if( m_exportFileName.isEmpty() )	return 1;


	double wPhoton = ( inputAperture * irradiance ) / m_numberOfRays;

	if( m_exportSurfaceName.isEmpty() )	return trf::ExportAll( m_exportFileName, wPhoton, m_photonMap );
	else
	{
		SoSearchAction* coinSearch = new SoSearchAction();
		coinSearch->setName( m_exportSurfaceName.toStdString().c_str() );
		coinSearch->setInterest( SoSearchAction::FIRST);
		coinSearch->apply( document.GetSceneKit() );

		SoPath* nodePath = coinSearch->getPath( );
		if( !nodePath ) return 0;
		SoNodeKitPath* selectedNodePath = static_cast< SoNodeKitPath* > ( nodePath );
		if( !selectedNodePath ) return 0;

		QModelIndex nodeIndex = sceneModel.IndexFromPath( *selectedNodePath );
		InstanceNode* selectedSurface = sceneModel.NodeFromIndex( nodeIndex );

		if( m_exportSurfaceInGlobalCoordinates )	return trf::ExportSurfaceGlobalCoordinates( m_exportFileName, selectedSurface, wPhoton, m_photonMap );
		else	return trf::ExportSurfaceLocalCoordinates( m_exportFileName, selectedSurface, wPhoton, m_photonMap );
	}

	return 1;
}

/**
 * Compute a map with the InstanceNodes of sub-tree with top node \a instanceNode.
 *
 *The map stores for each InstanceNode its BBox and its transform in global coordinates.
 **/
void ScriptRayTracer::ComputeSceneTreeMap( InstanceNode* instanceNode, Transform parentWTO )
{
	if( !instanceNode ) return;
	SoBaseKit* coinNode = static_cast< SoBaseKit* > ( instanceNode->GetNode() );
	if( !coinNode ) return;

	if( coinNode->getTypeId().isDerivedFrom( TSeparatorKit::getClassTypeId() ) )
	{
		SoTransform* nodeTransform = static_cast< SoTransform* >(coinNode->getPart( "transform", true ) );
		Transform objectToWorld = tgf::TransformFromSoTransform( nodeTransform );
		Transform worldToObject = objectToWorld.GetInverse();

		BBox nodeBB;
		Transform nodeWTO(worldToObject * parentWTO );
		instanceNode->SetIntersectionTransform( nodeWTO );

		for( int index = 0; index < instanceNode->children.count() ; ++index )
		{
			InstanceNode* childInstance = instanceNode->children[index];
			ComputeSceneTreeMap(childInstance, nodeWTO );

			nodeBB = Union( nodeBB, childInstance->GetIntersectionBBox() );
		}

		instanceNode->SetIntersectionBBox( nodeBB );

	}
	else
	{
		Transform shapeTransform = parentWTO;
		Transform shapeToWorld = shapeTransform.GetInverse();
		BBox shapeBB;

		if(  instanceNode->children.count() > 0 )
		{
			InstanceNode* shapeInstance = 0;
			if( instanceNode->children[0]->GetNode()->getTypeId().isDerivedFrom( TShape::getClassTypeId() ) )
				shapeInstance =  instanceNode->children[0];
			else if(  instanceNode->children.count() > 1 )	shapeInstance =  instanceNode->children[1];

			SoGetBoundingBoxAction* bbAction = new SoGetBoundingBoxAction( SbViewportRegion() ) ;
			coinNode->getBoundingBox( bbAction );

			SbBox3f box = bbAction->getXfBoundingBox().project();
			delete bbAction;

			SbVec3f pMin = box.getMin();
			SbVec3f pMax = box.getMax();
			shapeBB = shapeToWorld( BBox( Point3D( pMin[0],pMin[1],pMin[2]), Point3D( pMax[0],pMax[1],pMax[2]) ) );
		}
		instanceNode->SetIntersectionTransform( shapeTransform );

		instanceNode->SetIntersectionBBox( shapeBB );
	}

}
