/*  ******************************************************************************* **
 * GIGC - Grupo de Investigaci�n de Gr�ficos por Computadora - UTN FRBA - Argentina
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  **
 * <copyright> copyright (c) 2013 </copyright>	
 * <autor> Leandro Roberto Barbagallo </autor>
 * <license href="https://github.com/gigc/Janua">MIT</license>
 ********************************************************************************** */

#pragma once

#include "Vector3f.h"

//Portal quadrilateral in Counter Clock wise order.
//eg.
//     d ------- c
//		|		| 
//		|		|
//	   a ------- b	
class PortalQuad
{
public:
	PortalQuad(const Vector3f points[4]);

	Vector3f points[4];

	~PortalQuad(void);
};

