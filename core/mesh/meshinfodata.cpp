//
// GigaMesh - The GigaMesh Software Framework is a modular software for display,
// editing and visualization of 3D-data typically acquired with structured light or
// structure from motion.
// Copyright (C) 2009-2020 Hubert Mara
//
// This file is part of GigaMesh.
//
// GigaMesh is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GigaMesh is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
//

#include <GigaMesh/mesh/meshinfodata.h>

#include <sstream>      // std::stringstream
#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision
#include <iostream>     // std::cout
#include <string>       // std::string, std::to_string
#include <math.h>       // sqrt

#include <GigaMesh/mesh/gmcommon.h>

//! Constructer calls MeshInfoData::reset() and sets the names for the enumerators.
MeshInfoData::MeshInfoData() {
	// String names
	mStringName[FILENAME]             = "Filename";
	mStringName[MODEL_ID]             = "Model Id";
	mStringName[MODEL_MATERIAL]       = "Model Material";
	mStringName[MODEL_WEBREFERENCE]   = "Web-Reference";

	// Unsigned long names
	mCountULongName[VERTICES_TOTAL] = "Total number of vertices";
	mCountULongName[VERTICES_NAN] = "Vertices not-a-number";
	mCountULongName[VERTICES_NORMAL_LEN_NORMAL] = "Vertex normal vector length normal";
	mCountULongName[VERTICES_SOLO] = "Vertices solo";
	mCountULongName[VERTICES_POLYLINE] = "Vertices of polylines";
	mCountULongName[VERTICES_BORDER] = "Border vertices";
	mCountULongName[VERTICES_NONMANIFOLD] = "Vertices non-manifold";
	mCountULongName[VERTICES_ON_INVERTED_EDGE] = "Vertices along an inverted edge";
	mCountULongName[VERTICES_PART_OF_ZERO_FACE] = "Vertices part of a zero area face";
	mCountULongName[VERTICES_SYNTHETIC] = "Vertices synthetic";
	mCountULongName[VERTICES_MANUAL] = "Vertices manually added";
	mCountULongName[VERTICES_CIRCLE_CENTER] = "Vertices circle center";
	mCountULongName[VERTICES_SELECTED] = "Vertices selected";
	mCountULongName[VERTICES_FUNCVAL_FINITE] = "Vertices with finite function value";
	mCountULongName[VERTICES_FUNCVAL_LOCAL_MIN] = "Vertices with local function value minimum";
	mCountULongName[VERTICES_FUNCVAL_LOCAL_MAX] = "Vertices with local function value maximum";
	mCountULongName[FACES_TOTAL] = "Total number of faces";
	mCountULongName[FACES_SOLO] = "Solo faces";
	mCountULongName[FACES_BORDER] = "Border faces";
	mCountULongName[FACES_BORDER_THREE_VERTICES] = "Faces with three border vertices";
	mCountULongName[FACES_BORDER_BRDIGE_TRICONN] = "Border faces bridge tri-connection";
	mCountULongName[FACES_BORDER_BRDIGE] = "Bridge border faces";
	mCountULongName[FACES_BORDER_DANGLING] = "Dangling border faces";
	mCountULongName[FACES_MANIFOLD] = "Manifold faces";
	mCountULongName[FACES_NONMANIFOLD] = "Non-manifold faces";
	mCountULongName[FACES_STICKY] = "Sticky faces";
	mCountULongName[FACES_ZEROAREA] = "Faces with zero area";
	mCountULongName[FACES_INVERTED] = "Inverted Faces";
	mCountULongName[FACES_SELECTED] = "Selected Faces";
	mCountULongName[FACES_WITH_SYNTH_VERTICES] = "Faces with synthetic vertices";

	// Double names
	mmCountDoubleName[BOUNDINGBOX_MIN_X]    = "Minimum x coordinate";
	mmCountDoubleName[BOUNDINGBOX_MIN_Y]    = "Minimum y coordinate";
	mmCountDoubleName[BOUNDINGBOX_MIN_Z]    = "Minimum z coordinate";
	mmCountDoubleName[BOUNDINGBOX_MAX_X]    = "Maximum x coordinate";
	mmCountDoubleName[BOUNDINGBOX_MAX_Y]    = "Maximum y coordinate";
	mmCountDoubleName[BOUNDINGBOX_MAX_Z]    = "Maximum z coordinate";
	mmCountDoubleName[BOUNDINGBOX_WIDTH]    = "Bounding box width";
	mmCountDoubleName[BOUNDINGBOX_HEIGHT]   = "Bounding box height";
	mmCountDoubleName[BOUNDINGBOX_THICK]    = "Bounding box thickness";
	mmCountDoubleName[TOTAL_AREA]           = "Total area";
	mmCountDoubleName[TOTAL_VOLUME_DX]      = "Total volume (dx)";
	mmCountDoubleName[TOTAL_VOLUME_DY]      = "Total volume (dy)";
	mmCountDoubleName[TOTAL_VOLUME_DZ]      = "Total volume (dz)";

	reset();
}

//! Destructor.
MeshInfoData::~MeshInfoData() {
	// Nothing
}

//! Reset all values.
void MeshInfoData::reset() {
	for( std::string& countValue : this->mStrings ) {
		countValue = "";
	}
	for( auto& countValue : this->mCountULong ) {
		countValue = _NOT_A_NUMBER_ULONG_;
	}
	for( double& countValue : this->mCountDouble ) {
		countValue = _NOT_A_NUMBER_DBL_;
	}
}

//! Format mesh information as HTML.
//!
//! See Mesh::dumpMeshInfo for a related plain text method.
//!
//! @returns false in case of an error. True otherwise.
bool MeshInfoData::getMeshInfoHTML(
        std::string&          rInfoHTML    //!< Output: String to be passed e.g. for Qt Infobox
) {
	// Compute and format relative amount
	//-----------------------------------------------------------
	std::stringstream fractionsFormatted[MeshInfoData::ULONG_COUNT];
	for( int i=0; i<=MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX; i++ ) {
		fractionsFormatted[i] << std::fixed << std::setprecision(2) << this->mCountULong[i]*100.0 / this->mCountULong[MeshInfoData::VERTICES_TOTAL];
	}
	for( int i=MeshInfoData::FACES_SOLO; i<MeshInfoData::ULONG_COUNT; i++ ) {
		fractionsFormatted[i] << std::fixed << std::setprecision(2) << this->mCountULong[i]*100.0 / this->mCountULong[MeshInfoData::FACES_TOTAL];
	}

	// Compute the area and the resolution of the mesh
	//-----------------------------------------------------------
	double areaAcq = this->mCountDouble[MeshInfoData::TOTAL_AREA];
	std::string areaAcqStr   = std::to_string( areaAcq );
	std::string avgResMetric = std::to_string( this->mCountULong[MeshInfoData::VERTICES_TOTAL]/areaAcq );
	std::string avgResDPI    = std::to_string( 2.54*sqrt(this->mCountULong[MeshInfoData::VERTICES_TOTAL]/areaAcq) );
	// Format the numbers
	std::size_t foundDot = areaAcqStr.find('.');
	if( foundDot > 4 ) { // Cut-off fractions for large values
		areaAcqStr = areaAcqStr.substr( 0 , foundDot );
	} else { // Show only 5 digits including the fraction
		areaAcqStr = areaAcqStr.substr( 0 , 6 );
	}
	foundDot = avgResMetric.find('.');
	if( foundDot > 4 ) { // Cut-off fractions for large values
		avgResMetric = avgResMetric.substr( 0 , foundDot );
	} else { // Show only 5 digits including the fraction
		avgResMetric = avgResMetric.substr( 0 , 6 );
	}
	foundDot = avgResDPI.find('.');
	if( foundDot > 4 ) { // Cut-off fractions for large values
		avgResDPI = avgResDPI.substr( 0 , foundDot );
	} else { // Show only 5 digits including the fraction
		avgResDPI = avgResDPI.substr( 0 , 6 );
	}
	//-----------------------------------------------------------

	std::string tableBorder = "0"; // For visual debugging set to 1 - 0 (zero) for release!
	std::string infoStr = "<!DOCTYPE html>\n";
	infoStr += "<html>\n";
	infoStr += "<head>\n";
	infoStr += "<title>GigaMesh Information about [" + this->mStrings[MeshInfoData::FILENAME] + "]</title>\n";
	infoStr += "</head>\n";
	infoStr += "<body>\n";

	infoStr += "<b>Filename:</b> " + this->mStrings[MeshInfoData::FILENAME] + "<br />\n";

	// Outer table - Row I, Col I
	infoStr += "<table align='center' border='" + tableBorder + "'>\n";
	infoStr += "<tr><td align='center'>\n";

	infoStr += "<b>Bounding Box</b> in mm (assumed)\n";
	infoStr += "<table border='" + tableBorder + "'>\n";
	infoStr += "<tr><td>X:</td><td align='right'>" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_X] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;&#8722;&nbsp;</td>";
	infoStr += "<td align='left'>" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_X] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;=&nbsp;</td>";
	infoStr += "<td align='center'>&nbsp;" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_X] - this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_X] ) + "</td></tr>\n";
	infoStr += "<tr><td>Y:</td><td align='right'>" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Y] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;&#8722;&nbsp;</td>";
	infoStr += "<td align='left'>" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Y] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;=&nbsp;</td>";
	infoStr += "<td align='center'>&nbsp;" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Y] - this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Y] ) + "</td></tr>\n";
	infoStr += "<tr><td>Z:</td><td align='right'>" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Z] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;&#8722;&nbsp;</td>";
	infoStr += "<td align='left'>" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Z] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;=&nbsp;</td>";
	infoStr += "<td align='center'>&nbsp;" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Z] - this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Z] ) + "</td></tr>\n";
	infoStr += "</table>\n";

	// Outer table - Row I, Col II
	infoStr += "</td><td>&nbsp;&nbsp;&nbsp;&nbsp;</td><td align='center'>\n";

	infoStr += "(units assumed)\n";
	infoStr += "<table align=\"center\" width=\"99%\" border='" + tableBorder + "'>\n";
	infoStr += "<tr>\n";
	infoStr += "<td align=\"left\"><b>Resolution,&nbsp;average:</b></td>";
	infoStr += "<td align=\"right\">" + avgResMetric + "</td>";
	infoStr += "<td align=\"left\">cm<sup>-2</sup></td>";
	infoStr += "</tr>\n";
	infoStr += "<tr>\n";
	infoStr += "<td align=\"left\"></td>";
	infoStr += "<td align=\"right\">" + avgResDPI + "</td>";
	infoStr += "<td align=\"left\">DPI</td>";
	infoStr += "</tr>\n";
	infoStr += "<tr>\n";
	infoStr += "<td align=\"left\"><b>Total&nbsp;surface area:</b></td>";
	infoStr += "<td align=\"right\">" + areaAcqStr + "</td>";
	infoStr += "<td align=\"left\">cm<sup>2</sup></td>";
	infoStr += "</tr>\n";
	infoStr += "</table>\n";

	// Outer table - Row II, Col I
	infoStr += "</td></tr>\n"
	           "<tr><td colspan='3'><hr /></tr>\n"
	           "<tr><td>\n";

	infoStr += "<table align=\"center\" width=\"99%\" border='" + tableBorder + "'>\n";
	infoStr += "<tr><td colspan=\"3\" align=\"center\"><b>Vertices</b></td></tr>\n";
	infoStr += "<tr><td>Total:</td><td align=\"right\">"                                + std::to_string( this->mCountULong[MeshInfoData::VERTICES_TOTAL] )                  + "</td><td align=\"center\">-</td></tr>\n";
	infoStr += "<tr><td>NaN<sup>a)</sup>&ensp;coordinate(s):</td><td align=\"right\">"  + std::to_string( this->mCountULong[MeshInfoData::VERTICES_NAN] )                    + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_SOLO].str()                   + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Normal&nbsp;length&nbsp;not&nbsp;normal:</td><td align=\"right\">"      + std::to_string( this->mCountULong[MeshInfoData::VERTICES_NORMAL_LEN_NORMAL] )      + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_NORMAL_LEN_NORMAL].str()      + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Solo:</td><td align=\"right\">"                                 + std::to_string( this->mCountULong[MeshInfoData::VERTICES_SOLO] )                   + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_SOLO].str()                   + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Polyline:</td><td align=\"right\">"                             + std::to_string( this->mCountULong[MeshInfoData::VERTICES_POLYLINE] )               + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_POLYLINE].str()               + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Border:</td><td align=\"right\">"                               + std::to_string( this->mCountULong[MeshInfoData::VERTICES_BORDER] )                 + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_BORDER].str()                 + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Non-manifold:</td><td align=\"right\">"                         + std::to_string( this->mCountULong[MeshInfoData::VERTICES_NONMANIFOLD] )            + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_NONMANIFOLD].str()            + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Inverted edge<sup>b)</sup>:</td><td align=\"right\">"           + std::to_string( this->mCountULong[MeshInfoData::VERTICES_ON_INVERTED_EDGE] )       + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_ON_INVERTED_EDGE].str()       + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Part&nbsp;of&nbsp;zero&nbsp;area&nbsp;face:</td><td align=\"right\">"   + std::to_string( this->mCountULong[MeshInfoData::VERTICES_PART_OF_ZERO_FACE] )      + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_PART_OF_ZERO_FACE].str()      + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Synthetic:</td><td align=\"right\">"                            + std::to_string( this->mCountULong[MeshInfoData::VERTICES_SYNTHETIC] )              + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_SYNTHETIC].str()              + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Manual:</td><td align=\"right\">"                               + std::to_string( this->mCountULong[MeshInfoData::VERTICES_MANUAL] )                 + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_MANUAL].str()                 + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Circle Centers:</td><td align=\"right\">"                       + std::to_string( this->mCountULong[MeshInfoData::VERTICES_CIRCLE_CENTER] )          + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_CIRCLE_CENTER].str()          + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Selected:</td><td align=\"right\">"                             + std::to_string( this->mCountULong[MeshInfoData::VERTICES_SELECTED] )               + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_SELECTED].str()               + "&#37;</td></tr>\n";
	infoStr += "<tr><td colspan=\"3\"><i><u>Function values:</u></i></td></tr>\n";
	infoStr += "<tr><td>...&ensp;non finite:</td><td align=\"right\">"                           + std::to_string( this->mCountULong[MeshInfoData::VERTICES_TOTAL] - this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_FINITE] )         + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_FUNCVAL_FINITE].str()         + "&#37;</td></tr>\n";
	infoStr += "<tr><td>...&ensp;local&ensp;minimum:</td><td align=\"right\">"                   + std::to_string( this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MIN] )      + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MIN].str()      + "&#37;</td></tr>\n";
	infoStr += "<tr><td>...&ensp;local&ensp;maximum:</td><td align=\"right\">"                   + std::to_string( this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX] )      + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX].str()      + "&#37;</td></tr>\n";
	infoStr += "</table>\n";

	// Outer table - Row II, Col II
	infoStr += "</td><td>&nbsp;&nbsp;&nbsp;&nbsp;</td><td>\n";

	infoStr += "<table align=\"center\" width=\"99%\" border='" + tableBorder + "'>\n";
	infoStr += "<tr><td colspan=\"3\" align=\"center\"><b>Faces</b></td></tr>\n";
	infoStr += "<tr><td>Total:</td><td align=\"right\">"                                       + std::to_string( this->mCountULong[MeshInfoData::FACES_TOTAL] )                   + "</td><td align=\"center\">-</td></tr>\n";
	infoStr += "<tr><td>Manifold:</td><td align=\"right\">"                                    + std::to_string( this->mCountULong[MeshInfoData::FACES_MANIFOLD] )                + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_MANIFOLD].str()              + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Non-manifold:</td><td align=\"right\">"                                + std::to_string( this->mCountULong[MeshInfoData::FACES_NONMANIFOLD] )             + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_NONMANIFOLD].str()           + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Inverted<sup>c)</sup>:</td><td align=\"right\">"                       + std::to_string( this->mCountULong[MeshInfoData::FACES_INVERTED] )                + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_INVERTED].str()              + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Sticky:</td><td align=\"right\">"                                      + std::to_string( this->mCountULong[MeshInfoData::FACES_STICKY] )                  + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_STICKY].str()                + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Zero&ensp;area:</td><td align=\"right\">"                              + std::to_string( this->mCountULong[MeshInfoData::FACES_ZEROAREA] )                + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_ZEROAREA].str()              + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Border:</td><td align=\"right\">"                                      + std::to_string( this->mCountULong[MeshInfoData::FACES_BORDER] )                  + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_BORDER].str()                + "&#37;</td></tr>\n";
	infoStr += "<tr><td>...&nbsp;3&nbsp;Vertices&nbsp;(3V):</td><td align=\"right\">"          + std::to_string( this->mCountULong[MeshInfoData::FACES_BORDER_THREE_VERTICES] )   + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_BORDER_THREE_VERTICES].str() + "&#37;</td></tr>\n";
	infoStr += "<tr><td>&nbsp;&nbsp;&nbsp;...&nbsp;Bridge&nbsp;triconn.&nbsp;(3V0E):</td><td align=\"right\">"   + std::to_string( this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE_TRICONN] )   + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_BORDER_BRDIGE_TRICONN].str() + "&#37;</td></tr>\n";
	infoStr += "<tr><td>&nbsp;&nbsp;&nbsp;...&nbsp;Bridge&nbsp;(3V1E):</td><td align=\"right\">"                 + std::to_string( this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE] )           + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_BORDER_BRDIGE].str()         + "&#37;</td></tr>\n";
	infoStr += "<tr><td>&nbsp;&nbsp;&nbsp;...&nbsp;Dangling&nbsp;(3V2E):</td><td align=\"right\">"               + std::to_string( this->mCountULong[MeshInfoData::FACES_BORDER_DANGLING] )         + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_BORDER_DANGLING].str()       + "&#37;</td></tr>\n";
	infoStr += "<tr><td>&nbsp;&nbsp;&nbsp;...&nbsp;3&nbsp;Edges&nbsp;(Solo,3E):</td><td align=\"right\">"        + std::to_string( this->mCountULong[MeshInfoData::FACES_SOLO] )                    + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_SOLO].str()                  + "&#37;</td></tr>\n";
	//infoStr += "<tr><td></td><td></td><td></td></tr>\n"; // Empty line
	infoStr += "<tr><td>Selected:</td><td align=\"right\">"                                    + std::to_string( this->mCountULong[MeshInfoData::FACES_SELECTED] )                + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_SELECTED].str()              + "&#37;</td></tr>\n";
	infoStr += "</table>\n";

	// Outer table - End
	infoStr += "</td></tr>\n";
	infoStr += "</table>\n";

	//! \todo provide more information about polylines - maybe using another dialog.
	//infoStr += "<p align=\"center\"><b>Polylines:</b>&nbsp;" + std::to_string( getPolyLineNr() ) + "</p>\n";

	// Footnotes
	infoStr += "<p align='left'>";
	infoStr += "<i>a)</i> Not a Number.<br />\n";
	infoStr += "<i>b)</i> Some non-manifold edges are counted as inverted.<br />\n";
	infoStr += "<i>c)</i> Contains non-manifold faces.<br />\n";
	infoStr += "</p>";

	infoStr += "</body>\n";
	infoStr += "</html>\n";
	// std::cout << infoStr << std::endl; // For debugging the HTML code.

	rInfoHTML = infoStr;
	return( true );
}

bool MeshInfoData::getMeshInfoPropertyName(
                const MeshInfoData::eMeshPropertyString rPropId,
                std::string& rPropName
) {
	if( rPropId == STRING_COUNT ) {
		return( false );
	}
	rPropName = mStringName[rPropId];
	return( true );
}


bool MeshInfoData::getMeshInfoPropertyName(
                const MeshInfoData::eMeshPropertyULongCount rPropId,
                std::string& rPropName
) {
	if( rPropId == ULONG_COUNT ) {
		return( false );
	}
	rPropName = mCountULongName[rPropId];
	return( true );
}


bool MeshInfoData::getMeshInfoPropertyName(
                const MeshInfoData::eMeshPropertyDouble rPropId,
                std::string& rPropName
) {
	if( rPropId == DOUBLE_COUNT ) {
		return( false );
	}
	rPropName = mmCountDoubleName[rPropId];
	return( true );
}
