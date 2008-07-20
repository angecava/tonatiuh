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

#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoGLTextureCoordinateElement.h>
#include <Inventor/elements/SoMaterialBindingElement.h>

#include "DifferentialGeometry.h"
#include "ShapeParabolicDisk.h"
#include "tgf.h"
#include "tgc.h"
#include "Vector3D.h"

using tgc::Pi;
using tgc::TwoPi;
using namespace std;

SO_NODE_SOURCE(ShapeParabolicDisk);

void ShapeParabolicDisk::initClass()
{
	SO_NODE_INIT_CLASS(ShapeParabolicDisk, TShape, "TShape");
}

ShapeParabolicDisk::ShapeParabolicDisk()
{
	SO_NODE_CONSTRUCTOR(ShapeParabolicDisk);
	SO_NODE_ADD_FIELD(m_focus, (1.0));
	SO_NODE_ADD_FIELD(m_radius, (10.0));
	SO_NODE_ADD_FIELD(m_phiMax, (1.5*Pi));
}

ShapeParabolicDisk::~ShapeParabolicDisk()
{

}

void ShapeParabolicDisk::generatePrimitives(SoAction *action)
{
    SoPrimitiveVertex   pv;

    // Access the state from the action.
    SoState  *state = action->getState();

    // See if we have to use a texture coordinate function,
    // rather than generating explicit texture coordinates.
    SbBool useTexFunc = ( SoTextureCoordinateElement::getType(state) ==
                          SoTextureCoordinateElement::FUNCTION );

    // If we need to generate texture coordinates with a
    // function, we'll need an SoGLTextureCoordinateElement.
    // Otherwise, we'll set up the coordinates directly.
    const SoTextureCoordinateElement* tce;
    SbVec4f texCoord;
    if ( useTexFunc ) tce = SoTextureCoordinateElement::getInstance(state);
    else
    {
        texCoord[2] = 0.0;
        texCoord[3] = 1.0;
    }

    // Determine if there's a material bound per part.
    SoMaterialBindingElement::Binding binding = SoMaterialBindingElement::get(state);
    SbBool materialPerPart = ( binding == SoMaterialBindingElement::PER_PART ||
                               binding == SoMaterialBindingElement::PER_PART_INDEXED);

    // We'll use this macro to make the code easier. It uses the
    // "point" variable to store the primitive vertex's point.
    SbVec3f  point;


	const int rows = 50; // Number of points per row
    const int columns = 50; // Number of points per column
    const int totalPoints = (rows)*(columns); // Total points in the grid

    float vertex[totalPoints][6];

    int h = 0;
    double ui = 0;
	double vj = 0;

	//double u_step = 1.0 /(double)(rows-1); // Size of the step in parameter u to go from 0 to 1 in the number of rows
	//double v_step = 1.0 /(double)(columns-1); // Size of the step in parameter v to go from 0 to 1 in the number of columns

    for (int i = 0; i < rows; i++)
    {
    	ui =( 1.0 /(double)(rows-1) ) * i;

    	for ( int j = 0 ; j < columns ; j++ )
    	{

    		vj = ( 1.0 /(double)(columns-1) ) * j;

    		Point3D point = GetPoint3D(ui, vj);
    		SbVec3f normal = GetNormal(ui, vj);

    		vertex[h][0] = point.x;
    		vertex[h][1] = point.y;
    		vertex[h][2] = point.z;
    		vertex[h][3] = normal[0];
    		vertex[h][4] = normal[1];
    		vertex[h][5] = normal[2];

    		pv.setPoint( vertex[h][0], vertex[h][1], vertex[h][2] );
    		h++; //Increase h to the next point.
    		//vj += v_step; // Increase parameter vj to go from initial 0 to 1 at the end of the row (keep the same ui for the whole row)

    	}
    	//vj = 0; // Re-initialize parameter vj to 0 in order to start from 0 in the next row
		//ui += u_step; // Increase parameter ui to go from initial 0 to 1 in the last row.

    }

	const int totalIndices  = (rows-1)*(columns-1)*4;
    int32_t* indices = new int32_t[totalIndices];
    int k = 0;
    for(int irow = 0; irow < (rows-1); irow++)
           for(int icolumn = 0; icolumn < (columns-1); icolumn++)
           {
           	indices[k] = irow*columns + icolumn;
        	indices[k+1] = indices[k] + 1;
        	indices[k+3] = indices[k] + columns;
        	indices[k+2] = indices[k+3] + 1;

        	k+=4; //Set k to the first point of the next face.
           }

    float finalvertex[totalIndices][6];
    for(int ivert = 0; ivert<totalIndices;ivert++)
    {
    	finalvertex[ivert][0] = vertex[indices[ivert]][0];
    	finalvertex[ivert][1] = vertex[indices[ivert]][1];
    	finalvertex[ivert][2] = vertex[indices[ivert]][2];
    	finalvertex[ivert][3] = vertex[indices[ivert]][3];
    	finalvertex[ivert][4] = vertex[indices[ivert]][4];
    	finalvertex[ivert][5] = vertex[indices[ivert]][5];
    }
    delete[] indices;
#define GEN_VERTEX(pv, x, y, z, s, t, normal)   \
     point.setValue( x,                         \
                     y,                         \
                     z);                        \
     if (useTexFunc)                            \
       texCoord = tce->get(point, normal);      \
     else {                                     \
       texCoord[0] = s;                         \
       texCoord[1] = t;                         \
     }                                          \
     pv.setPoint(point);                        \
     pv.setNormal(normal);                      \
     pv.setTextureCoords(texCoord);             \
     shapeVertex(&pv)


    float u = 1;
    float v = 1;

	beginShape(action, QUADS );
    for( int i = 0; i < totalIndices; i++ )
    {
    	SbVec3f normal(finalvertex[i][3],finalvertex[i][4], finalvertex[i][5] );
    	GEN_VERTEX(pv,  finalvertex[i][0], finalvertex[i][1],  finalvertex[i][2], u,  v, normal);
    }
    endShape();
}

void ShapeParabolicDisk::computeBBox(SoAction *, SbBox3f &box, SbVec3f &center)
{
	double cosPhiMax = cos( m_phiMax.getValue() );
	double sinPhiMax = sin( m_phiMax.getValue() );

	double xmin = ( m_phiMax.getValue() >= Pi ) ? -m_radius.getValue() : m_radius.getValue() * cosPhiMax;
	double xmax = m_radius.getValue();
	double ymin = 0.0;
	if( m_phiMax.getValue() > Pi ) ymin = ( m_phiMax.getValue() < 1.5*Pi ) ? m_radius.getValue() * sinPhiMax : -m_radius.getValue();
	double ymax = ( m_phiMax.getValue() < Pi/2.0 )? m_radius.getValue() * sinPhiMax : m_radius.getValue();

	double zmax = m_radius.getValue()*m_radius.getValue()/(4*m_focus.getValue());
	double zmin = 0.0;
	box.setBounds(SbVec3f( xmin, ymin, zmin), SbVec3f( xmax, ymax, zmax));
}

bool ShapeParabolicDisk::Intersect(const Ray& objectRay, double* tHit, DifferentialGeometry* dg) const
{

	Vector3D vObjectRayOrigin = Vector3D( objectRay.origin );
	double A = objectRay.direction.x*objectRay.direction.x + objectRay.direction.y*objectRay.direction.y;
    double B = 2.0 * ( objectRay.direction.x* objectRay.origin.x + objectRay.direction.y*objectRay.origin.y - 2 * m_focus.getValue() * objectRay.direction.z);
	double C = objectRay.origin.x * objectRay.origin.x + objectRay.origin.y * objectRay.origin.y - 4 * m_focus.getValue() * objectRay.origin.z;

	// Solve quadratic equation for _t_ values
	double t0, t1;
	if( !tgf::Quadratic( A, B, C, &t0, &t1 ) ) return false;

	// Compute intersection distance along ray
	if( t0 > objectRay.maxt || t1 < objectRay.mint ) return false;
    double thit = ( t0 > objectRay.mint )? t0 : t1 ;
    if( thit > objectRay.maxt ) return false;

    //Evaluate Tolerance
	double tol = 0.00001;
	if( (thit - objectRay.mint) < tol ) return false;

	//Compute possible hit position
	Point3D hitPoint = objectRay( thit );

    // Test intersection against clipping parameters

	if( sqrt(hitPoint.x*hitPoint.x + hitPoint.y*hitPoint.y) > m_radius.getValue())
		{
			if ( thit == t1 ) return false;
			if ( t1 > objectRay.maxt ) return false;
			thit = t1;

			hitPoint = objectRay( thit );

			if( sqrt(hitPoint.x*hitPoint.x + hitPoint.y*hitPoint.y) > m_radius.getValue()) return false;
		}

	// Now check if the function is being called from IntersectP,
	// in which case the pointers tHit and dg are 0
	if( ( tHit == 0 ) && ( dg == 0 ) ) return true;
	else if( ( tHit == 0 ) || ( dg == 0 ) ) tgf::SevereError( "Function ParabolicCyl::Intersect(...) called with null pointers" );

///////////////////////////////////////////////////////////////////////////////////////

	// Compute definitive Circular Parabolic Facet hit position
    hitPoint = objectRay( thit );
	double phi = atan2( hitPoint.y, hitPoint.x );
	if ( phi < 0. ) phi += TwoPi;
	double radius = sqrt(hitPoint.x*hitPoint.x + hitPoint.y*hitPoint.y);

	// Find parametric representation of paraboloid hit
	double u = phi / m_phiMax.getValue();
	double v = radius /m_radius.getValue();

	// Compute Circular Parabolic Facet \dpdu and \dpdv

	Vector3D dpdu( -radius*sin(phi), radius*cos(phi), 0.0 );
	Vector3D dpdv( cos (phi), sin (phi), radius/(2*m_focus.getValue()) );

	// Compute Circular Parabolic Facet \dndu and \dndv
	Vector3D d2Pduu ( -radius*cos(phi), -radius*sin(phi), 0.0 );
	Vector3D d2Pduv ( -sin(phi), cos(phi), 0.0 );
	Vector3D d2Pdvv ( 1.0, 1.0, 1.0/(2.0*m_focus.getValue()) );

	// Compute coefficients for fundamental forms
	double E = DotProduct(dpdu, dpdu);
	double F = DotProduct(dpdu, dpdv);
	double G = DotProduct(dpdv, dpdv);
	Vector3D N = Normalize(CrossProduct(dpdu, dpdv));
	double e = DotProduct(N, d2Pduu);
	double f = DotProduct(N, d2Pduv);
	double g = DotProduct(N, d2Pdvv);

	// Compute \dndu and \dndv from fundamental form coefficients
	double invEGF2 = 1.0 / (E*G - F*F);
	Vector3D dndu = (f*F - e*G) * invEGF2 * dpdu +
		(e*F - f*E) * invEGF2 * dpdv;
	Vector3D dndv = (g*F - f*G) * invEGF2 * dpdu +
		(f*F - g*E) * invEGF2 * dpdv;

	// Initialize _DifferentialGeometry_ from parametric information
	*dg = DifferentialGeometry(hitPoint,
	                           dpdu,
							   dpdv,
	                           dndu,
							   dndv,
	                           u, v, this);


///////////////////////////////////////////////////////////////////////////////////////

	// Update _tHit_ for quadric intersection
	*tHit = thit;
	return true;
}

bool ShapeParabolicDisk::IntersectP( const Ray& worldRay ) const
{
	return Intersect( worldRay, 0, 0 );
}

Point3D ShapeParabolicDisk::Sample( double u, double v ) const
{
	return GetPoint3D( u, v );
}

bool ShapeParabolicDisk::OutOfRange( double u, double v ) const
{
	return ( ( u < 0.0 ) || ( u > 1.0 ) || ( v < 0.0 ) || ( v > 1.0 ) );
}

Point3D ShapeParabolicDisk::GetPoint3D (double u, double v) const
{
	if ( OutOfRange( u, v ) )	tgf::SevereError( "Function ShapeParabolicDisk::GetPoint3D called with invalid parameters" );

	double radius = u * m_radius.getValue();
	double phi = v * m_phiMax.getValue();

	double x = radius*cos (phi);
	double y = radius*sin (phi);
	double z = radius*radius/(4*m_focus.getValue());

	return Point3D (x, y, z);

}

SbVec3f ShapeParabolicDisk::GetNormal (double u, double v) const
{
	Point3D point = GetPoint3D( u, v );

	Vector3D r( 0, 0, 1 );
	Vector3D v1=Normalize(Vector3D( -point.x, -point.y, m_focus.getValue()-point.z ));
	Vector3D normal=Normalize( r+v1 );

	return SbVec3f ( normal.x, normal.y, normal.z );

}
