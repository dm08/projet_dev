//
//  histo_translation.cpp
//
//
//  Created by Jean-françois Villeforceix on 21/03/2016.
//
//
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <string>
#include <sstream>
#include <stdio.h>
#include <vector>

using namespace std;

vector<double> linspace(double a, double b, int n)
{
    vector<double> vect;
    double step = (b-a) / (n-1);

    while(a <= b) {
        vect.push_back(a);
        a += step;
    }
    return vect;
}

vector<vector<double>> imregionalmax(vector<vector<double>> matrice_histo)
{
    vector<vector<double>> matrice_histo_max;
    matrice_histo_max = matrice_histo;
    for(int i=1 ; i<matrice_histo.size()-1 ; i++)
    {
        for(int j=1 ; j<matrice_histo.size()-1 ; j++)
        {
            if(matrice_histo[i][j]<matrice_histo[i-1][j-1]
               || matrice_histo[i][j]<matrice_histo[i][j-1]
               || matrice_histo[i][j]<matrice_histo[i+1][j-1]
               || matrice_histo[i][j]<matrice_histo[i][j-1]
               || matrice_histo[i][j]<matrice_histo[i-1][j]
               || matrice_histo[i][j]<matrice_histo[i+1][j]
               || matrice_histo[i][j]<matrice_histo[i-1][j+1]
               || matrice_histo[i][j]<matrice_histo[i][j+1]
               || matrice_histo[i][j]<matrice_histo[i+1][j+1])
            {
                matrice_histo_max[i][j]=0;
            }
            matrice_histo_max[i][0]=0;
            matrice_histo_max[j][0]=0;
            matrice_histo_max[i][matrice_histo.size()-1]=0;
            matrice_histo_max[j][matrice_histo.size()-1]=0;
        }
    }
    return matrice_histo_max;
}

vector<vector<double>> find_non_zeros(vector<vector<double>> matrice_histo_max, int taille_vignette)
{
    vector<vector<double>> ligne_colonne;
    for(int i=1 ; i<matrice_histo_max.size()-1 ; i++)
    {
        for(int j=1 ; j<matrice_histo_max.size()-1 ; j++)
        {
            if(matrice_histo_max[i][j] != 0){
                vector<double> vtemp;
                vtemp = {i-taille_vignette,j-taille_vignette};
                ligne_colonne.push_back(vtemp);
            }
        }
    }
    return ligne_colonne;
}

int main(int argc, char** argv)
{
    string nomfichier{argv[1]};
    ifstream fichier(nomfichier, ios::in); // ouverture du fichier en lecture

    double x1, y1 ,x2 ,y2;

    vector<vector<double>> v1;
    vector<vector<double>> v2;

    if(fichier)     // si ouverture a reussi
    {
        string ligne;
        do{
            fichier >> x1 >> y1 >> x2 >> y2;
            //cout << xa << " " << ya << " " << xb << " " << yb << endl;
            vector<double> v;
            v.push_back(x1);
            v.push_back(y1);
            v1.push_back(v);
            v.clear();
            v.push_back(x2);
            v.push_back(y2);
            v2.push_back(v);
        }while(getline(fichier, ligne));
        //M.pop_back();
        fichier.close();
    }
    else{ //sinon
        cerr << "Impossible d'ouvrir le fichier !" << endl;
    }


    vector<double> dx;
    vector<double> dy;

    if(v1.size()==v2.size()){
        for(int i=0 ; i<v1.size() ; i++)
        {
            double temp;
            temp = v1[i][0]-v2[i][0];
            dx.push_back(temp);
            temp = v1[i][1]-v2[i][1];
            dy.push_back(temp);
        }
    }
    else{ //sinon
        cerr << "Les tableaux de points Harris n'ont pas la même taille !" << endl;
    }

    int taille_matrice = 40;
    double min_dx{*min_element(dx.begin(),dx.end())};
    double max_dx{*max_element(dx.begin(),dx.end())};
    double min_dy{*min_element(dy.begin(),dy.end())};
    double max_dy{*max_element(dy.begin(),dy.end())};
    vector<double> x = linspace(min_dx,max_dx,taille_matrice);
    vector<double> y = linspace(min_dy,max_dy,taille_matrice);

    vector<vector<double>> xy;

    for(int i=0 ; i<taille_matrice ; i++)
    {
        vector<double> vtemp;
        for(int j=0 ; j<taille_matrice ; j++)
        {
            vtemp.push_back(0);
        }
        xy.push_back(vtemp);
    }

    for(int i=0 ; i<v1.size() ; i++)
    {
        for(int j=0 ; j<x.size() ; j++)
        {
            for(int k=0 ; k<y.size() ; k++)
            {
                if(dx[i]>x[j] && dx[i]<=x[j+1])
                {
                    if(dy[i]>y[k] && dy[i]<=y[k+1])
                    {
                        xy[j][k] += 1;
                    }
                }
            }
        }
    }

    for(int i=0 ; i<xy.size() ; i++)
    {
        for(int j=0 ; j<xy.size() ; j++)
        {
            if(xy[i][j]<100)
            {
                xy[i][j]=0;
            }
        }
    }

    vector<vector<double>> xy_max = imregionalmax(xy);

    int taille_vignette = 20;
    vector<vector<double>> lc_nz = find_non_zeros(xy_max, taille_vignette);

    for(auto translation : lc_nz)
    {
        cout << "La translation (" << translation[0] << "," << translation[1] << ") a été détectée.";
        cout << endl;
    }

    return 0;
}
