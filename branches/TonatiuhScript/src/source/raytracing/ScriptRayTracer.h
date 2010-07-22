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


#ifndef SCRIPTRAYTRACER_H_
#define SCRIPTRAYTRACER_H_

#include <QObject>
#include <QString>
#include <QVector>

class InstanceNode;
class RandomDeviate;
class RandomDeviateFactory;
class TPhotonMap;
class TPhotonMapFactory;
class Transform;

class ScriptRayTracer : public QObject
{
	Q_OBJECT

public:
	ScriptRayTracer( QVector< TPhotonMapFactory* > listTPhotonMapFactory, QVector< RandomDeviateFactory* > listRandomDeviateFactory  );
	~ScriptRayTracer();

	void Clear();
	void SetExportFileName( QString filename );
	void SetExportSurfaceName( QString surfaceName );
	void SetExportSurfaceCoordinates( bool globalCoordinates );
	void SetTonatiuhModelFile ( QString filename );

	void SetNumberOfRays( double nrays );

	void SetPhotonMapType( QString typeName );

	void SetRandomDeviateType( QString typeName );

	void SetSunAzimtuh( double azimuth );
	void SetSunElevation( double elevation );
	void SetSunDistance( double distance );

	int Trace();

private:
	void ComputeSceneTreeMap( InstanceNode* instanceNode, Transform parentWTO );

	QString m_exportFileName;
	QString m_exportSurfaceName;
	bool m_exportSurfaceInGlobalCoordinates;
	QString m_modelFileName;
	unsigned long m_numberOfRays;


	QVector< TPhotonMapFactory* > m_TPhotonMapFactoryList;
	TPhotonMap* m_photonMap;

	QVector< RandomDeviateFactory* > m_RandomDeviateFactoryList;
	RandomDeviate* m_randomDeviate;

	double m_sunAzimuth;
	double m_sunElevation;
	double m_sunDistance;

};
#endif /* SCRIPTRAYTRACER_H_ */
