#pragma once

void ClearTriangles();
void DrawTriangles();
void RecursiveFractal(int k);
void ConstructiveFractals();
void MouseInteraction(double m_x, double m_y);
void AffineMatricesCalculation(double v_original[][2], double v_transformed[][2], double matrix[][3]);
void InverseMatrix(double original_m[][3], double inverse_m[][3]);