#include <voronoi\ch.h>

/*  determina los puntos extremos
    este algoritmo es muy lento, tiene una complejidad de O(n^4) */
void PuntosExtremosLento(Vertices *v, Modo modo, int retraso)
{
	/* este algoritmo hace lo que se le llama in-triangle test con
	cada punto en la nube, los puntos que se encuentren dentro de un
	triangulo son marcados como no extremos */
	/* para iterar en todos los puntos */
	int a, b, c, d;

	/* marcamos todos los puntos como extremos */
	for(a = 0; a < v->longitud; a++)
		v->elementos[a].extremo = true;
	
	/* ahora empezamos a iterar en los posibles triangulos */
	for(a = 0; a < v->longitud; a++)
	{
		for(b = a + 1; b < v->longitud; b++)
		{
			for(c = b + 1; c < v->longitud; c++)
			{

				if(modo == descriptivo)
				{
					DibujarSegmentoApartirDeVertices(&v->elementos[a], &v->elementos[b], VCOLOR_TRIANGULO_ACTUAL);
					DibujarSegmentoApartirDeVertices(&v->elementos[b], &v->elementos[c], VCOLOR_TRIANGULO_ACTUAL);
					DibujarSegmentoApartirDeVertices(&v->elementos[c], &v->elementos[a], VCOLOR_TRIANGULO_ACTUAL);
				}

				for(d = 0; d < v->longitud; d++)
				{
					/* evitamos revisar los puntos que forman el triangulo actual o los
						puntos extremos */
					if(d == a || d == b || d == c || !v->elementos[d].extremo)
						continue;

					if(modo == descriptivo)
					{
						DibujarVertice(&v->elementos[d], VCOLOR_PUNTO_ACTUAL);
						delay(retraso);

						DibujarPuntosExtremos(v, VCOLOR_PUNTO_EXTREMO, VCOLOR_PUNTO_DESCARTADO);
					}

					/* revisamos si el punto d se encuentra en el triangulo abc, si lo esta, entonces
						no es extremo */
					if( EnTriangulo(&v->elementos[a], &v->elementos[b], &v->elementos[c], &v->elementos[d]) )
					{
						v->elementos[d].extremo = false;
					}
				}

				if(modo == descriptivo)
				{
					DibujarSegmentoApartirDeVertices(&v->elementos[a], &v->elementos[b], getbkcolor());
					DibujarSegmentoApartirDeVertices(&v->elementos[b], &v->elementos[c], getbkcolor());
					DibujarSegmentoApartirDeVertices(&v->elementos[c], &v->elementos[a], getbkcolor());

					DibujarPuntosExtremos(v, VCOLOR_PUNTO_EXTREMO, VCOLOR_PUNTO_DESCARTADO);
				}
			}
		}
	}
}

/* determina la envolvente convexa buscando los lados extremos, en lugar de puntos,
este algoritmo es O(n^3) */
void LadosExtremos(Vertices *v, Modo modo, int retraso)
{
	int a, b;

	/* en este algoritmo hacemos lo contrario, todos son no extremos 
		hasta probar lo contrario */
	for(a = 0; a < v->longitud; a++)
		v->elementos[a].extremo = false;

	/* lo que hace es tomar dos puntos y revisar en que lado se encuentran
		los demas puntos, si todos los puntos estan a la izquierda, entonces,
		es un lado extremo, mucho mas rapido que puntos extremons, pero sigue
		siendo muy lento */
	for(a = 0; a < v->longitud; a++)
	{
		for(b = a + 1; b < v->longitud; b++)
		{
			RevisarLado(v, a, b, modo, retraso);
		}
	}
}

/* utiliza el algoritmo de la marcha de jarvis O(n^2) o envoltura de regalo */
void JarvisMarch(Vertices *v, Modo modo, int retraso)
{
	/* ahora calculamos el primer punto extremo */
	int ltl;
	int s, t, k;

	/* empezamos por marcarlos a todos como no extremos */
	for(k = 0; k < v->longitud; k++)
		v->elementos[k].extremo = false;

	/* tomamos el punto mas bajo en ambas coordenadas */
	ltl = LTL(v);
	k = ltl;

	/* y buscamos los demas puntos extremos */
	do
	{
		v->elementos[k].extremo = true;
		s = -1;

		/* revisando cada punto */
		for(t = 0; t < v->longitud; t++) 
		{
			if( t != k && t != s && 
				(s == -1 || ! EnLaIzquierda(&v->elementos[k], &v->elementos[s], &v->elementos[t]) ) )
			{
				if(modo == descriptivo)
				{
					DibujarSegmentoApartirDeVertices(&v->elementos[k], &v->elementos[t], VCOLOR_LADO_EXTREMO);

					DibujarVertice(&v->elementos[t], VCOLOR_PUNTO_ACTUAL);
					delay(retraso);

					DibujarSegmentoApartirDeVertices(&v->elementos[k], &v->elementos[t], getbkcolor());

					DibujarPuntosExtremos(v, VCOLOR_PUNTO_EXTREMO, VCOLOR_PUNTO_DESCARTADO);
				}
				s = t;
			}
		}

		/* dibujando envolvente */
		DibujarSegmentoApartirDeVertices(&v->elementos[k], &v->elementos[s], VCOLOR_LADO_EXTREMO);

		v->elementos[k].sucesor = s;
		k = s;
	}while(k != ltl);

	if(modo == descriptivo)
		DibujarSegmentoApartirDeVertices(&v->elementos[ltl], &v->elementos[v->elementos[k].sucesor], VCOLOR_LADO_EXTREMO);
}

/* utiliza el algoritmo de graham para dibujar la envolvente convexa O(n^2) */
void GrahamScan(Vertices *v, Modo modo, int retraso)
{
	/* stacks */
	/* en el paso de inicializacion, despues de ordenar por angulo polar
	los primeros dos elementos son puestos en S y los otros n-2 puntos en T 
	en orden inverso de manera que el elemento 3 quede en el tope */
	Vertices s = newVertices(v->longitud);
	Vertices t = newVertices(v->longitud);

	int i;

	/* preprocesado */
	OrdenarPorAnguloPolar(v);

	/* agregando elementos a stack 
		agregando manualmente para eficiencia */
	StackVerticePush(&s, &v->elementos[0]);
	StackVerticePush(&s, &v->elementos[1]);

	if(modo == descriptivo)
	{
		DibujarVertice(&s.elementos[0], VCOLOR_PUNTO_EXTREMO);
		DibujarVertice(&s.elementos[1], VCOLOR_PUNTO_EXTREMO);
		DibujarSegmentoApartirDeVertices(&s.elementos[0], &s.elementos[1], VCOLOR_LADO_EXTREMO);
	}

	/* agregando los otros n - 2 puntos 
		de manera inversa */
	for(i = v->longitud - 1; i > 1 ; i--)
		StackVerticePush(&t, &v->elementos[i]);

	/* empezamos el algoritmo de graham 
		consiste en agregar los puntos en T al stack S
		siempre y cuando se haga un giro hacia la izquierda,
		si no, se elimina el ultimo punto agragado al stack s */
	while(!t.vacio)
	{
		if(EnLaIzquierda(&s.elementos[s.stackIndice - 1], &s.elementos[s.stackIndice], &t.elementos[t.stackIndice]))
		{
			StackVerticePush(&s, StackVerticePop(&t));
		}
		else
		{
			if(modo == descriptivo)
			{
				DibujarSegmentoApartirDeVertices(&s.elementos[s.stackIndice], &s.elementos[s.stackIndice - 1], getbkcolor());
				DibujarVertice(&s.elementos[s.stackIndice], VCOLOR_PUNTO_DESCARTADO);
			}

			StackVerticePop(&s);
		}

		if(modo == descriptivo)
		{
			DibujarVertice(&s.elementos[s.stackIndice], VCOLOR_PUNTO_EXTREMO);
			DibujarSegmentoApartirDeVertices(&s.elementos[s.stackIndice], &s.elementos[s.stackIndice - 1], VCOLOR_LADO_EXTREMO);

			DibujarVertice(&t.elementos[t.stackIndice], VCOLOR_PUNTO_ACTUAL);

			if(!t.vacio)
				DibujarSegmentoApartirDeVertices(&s.elementos[s.stackIndice], &t.elementos[t.stackIndice], VCOLOR_TRIANGULO_ACTUAL);
				
			delay(retraso);

			if(!t.vacio)
				DibujarSegmentoApartirDeVertices(&s.elementos[s.stackIndice], &t.elementos[t.stackIndice], getbkcolor());
		}
	}

	if(modo != descriptivo)
		DibujarPoligonoDeStack(&s);
	else
	{
		DibujarPuntosDeStack(&s, VCOLOR_PUNTO_EXTREMO);
		DibujarSegmentoApartirDeVertices(&s.elementos[0], &s.elementos[s.stackIndice], VCOLOR_LADO_EXTREMO);
	}

	/* limpiando memoria utilizada */
	RemoverVertices(&s);
	RemoverVertices(&t);
}
