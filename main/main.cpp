/***************************************************************************
 *   Copyright (C) 2008 by Paul Lutus                                      *
 *   lutusp@arachnoid.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <QCoreApplication>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>>
#include "Complex.h"
#include "FFT.h"
#include "DFT.h"
#include <cmath>

#define FREQ_HZ     (unsigned int)500
#define FREQ_SAMPLE (unsigned int)(2.1*FREQ_HZ)
#define PRECISION   (unsigned int)(6)
#define NUMBER_PRECISION (10^PRECISION)
#define N           (unsigned int)1024
#define roundz(x,d) ((floor(((x)*pow(10,d))+.5))/pow(10,d))
using namespace std;

class fft_processor {
    bool inverse;
    bool dft_mode;

    FFT *fft;
    DFT *dft;

public:

    fft_processor() {
        fft = NULL;
        dft = NULL;
        inverse = false;
        dft_mode = false;
    }

    ~fft_processor() {
        delete fft;
        delete dft;
    }

    template <class T> void scan_string(T& t,char* s) {
        stringstream ss(s);
        ss >> t;
    }

    bool decode_comline(int argc,char **argv)
    {
        for(int i = 1;i < argc;i++) {
            string arg = (string) argv[i];
            if(arg.length() == 2 && arg[0] == '-') {
                char c = arg[1];
                switch (c) {
                case 'h':
                    cerr << "Usage: [-i (inverse FFT/DFT)]" << endl;
                    cerr << "       [-d (DFT instead of FFT)]" << endl;
                    return true;
                case 'i':
                    inverse = true;
                    break;
                case 'd':
                    dft_mode = true;
                    break;
                }
            }
        }
        return false;
    }

    void process(int argc, char **argv)
    {
        /*if(decode_comline(argc,argv)) {
            return;
        }*/

        FILE *fpFile;
        fpFile = fopen(argv[1],"r");
        if(fpFile == NULL)
        {
            perror("Error opening file");
            return;
        }


        unsigned long array_size,samples_per_second;
        char buffer[2048];
        char *ptr = &buffer[0];
        char c;
        memset(buffer,0x00,sizeof(buffer));
        while(1)
        {
            fread(&c, sizeof(char),1, fpFile);
            if((c != 0x00)&& (c != '\n'))
            {
                *ptr = c;
                ptr++;
            }
            else
            {
                array_size = atoi(buffer);
                break;
            }
        }


        memset(buffer,0x00,sizeof(buffer));
        ptr = &buffer[0];
        while(1)
        {
            fread(&c, sizeof(char),1, fpFile);
            if((c != 0x00) && (c != '\n'))
            {
                *ptr = c;
                ptr++;
            }
            else
            {
                samples_per_second = atoi(buffer);
                break;
            }
        }

        samples_per_second = FREQ_SAMPLE;
        array_size = N;
        dft_mode = 0;

        //while(array_size && samples_per_second)
        //{
            if(dft_mode)
            {
                if(dft)
                {
                    dft->resize(array_size);
                }
                else
                {
                    dft = new DFT(array_size);
                }
                Complex* array_input = dft->array_input();
                Complex* array_output = dft->array_output();
                for(unsigned long i = 0;i < array_size;i++)
                {
                    memset(buffer,0x00,sizeof(buffer));
                    ptr = &buffer[0];
                    while(1)
                    {
                        fread(&c, sizeof(char),1, fpFile);
                        if((c != 0x00) && (c != '\t'))
                        {
                            *ptr = c;
                            ptr++;
                        }
                        else
                        {
                            array_input[i].re = atoi(buffer);
                            break;
                        }
                    }
                }
                dft->dft1(inverse);
                cout << array_size << endl;
                cout << samples_per_second << endl;
                for(unsigned int i = 0;i < array_size;i++)
                {
                    cout << array_output[i].re << " " << array_output[i].im << endl;
                }
            } // if dft mode
            else
            { // fft mode
                if(fft)
                {
                    fft->resize(array_size);
                }
                else
                {
                    fft = new FFT(array_size);
                }
                Complex* array_input = fft->array_input();
                Complex** array_output = fft->array_output();

                volatile double f;
#if 0
                for(unsigned long i = 0;i <array_size;i++)
                {
                    memset(buffer,0x00,sizeof(buffer));
                    ptr = &buffer[0];
                    while(1)
                    {
                        fread(&c, sizeof(char),1, fpFile);
                        if((c != 0x00) && (c != '\t'))
                        {
                            *ptr = c;
                            ptr++;
                        }
                        else
                        {
                            size_t len = strlen(buffer);
                            char* temp = (char*)malloc(len+1);
                            strcpy(temp,buffer);
                            f = atof(temp);
                            array_input[i].re = f;
                            free(temp);
                            break;
                        }
                    }
#endif
                float t = 0;
                float ft,d;
                float fln = 0;

                cout << "***********ORIGINAL SIGNAL REAL AND IMAGINARY*************" << endl;
                for(unsigned int i = 0;i < array_size;i++)
                {
                    d = 2*M_PI*FREQ_HZ;
                    d *= t;
                    /*d = floor(NUMBER_PRECISION*d)/NUMBER_PRECISION;*/
                    ft = sin(d);
                    //ft = ceilf((NUMBER_PRECISION*ft)/NUMBER_PRECISION);
                    //ft = roundz(ft,PRECISION);
                    t = (float)(FREQ_SAMPLE);
                    t = (float)(1/t);
                    t *= (float)(i);
                    //t = roundz(t,PRECISION);
                    array_input[i].re = (float)(ft);
                    array_input[i].im = 0;
                    //cout << t << "\t" << array_input[i].re /*<< "\t" << array_input[i].im */<< endl;
                }
#if 0
                    memset(buffer,0x00,sizeof(buffer));
                    ptr = &buffer[0];
                    while(1)
                    {
                        fread(&c, sizeof(char),1, fpFile);
                        if((c != 0x00) && (c != '\n'))
                        {
                            *ptr = c;
                            ptr++;
                        }
                        else
                        {
                            size_t len = strlen(buffer);
                            char* temp = (char*)malloc(len+1);
                            strcpy(temp,buffer);
                            f = atof(temp);
                            array_input[i].im = f;
                            free(temp);
                            break;
                        }
                    }
                }
#endif
                fft->fft1(inverse);
                cout << array_size << endl;
                cout << samples_per_second << endl;

#if 0
                float t = 0;
                float ft,d;
                float fln = 0;
#endif
                cout << "***********FFT*************" << endl;
                for(unsigned int i = 0;i < array_size;i++)
                {
                    //cout << array_output[i]->re << "\t" << array_output[i]->im << endl;
                }
#if 0
                cout << "***********ORIGINAL SIGNAL*************" << endl;
                for(unsigned int i = 0;i < array_size;i++)
                {
                    d = 2*M_PI*FREQ_HZ;
                    d *= t;
                    d = floor(NUMBER_PRECISION*d)/NUMBER_PRECISION;
                    ft = sin(d*(3.14/180));
                    ft = floor(NUMBER_PRECISION*ft)/NUMBER_PRECISION;
                    t = 2*FREQ_HZ;
                    t = 1/t;
                    t *= i;
                    cout << t /*<< " " << d*/ << "\t" << ft << endl;
                }
#endif
                cout << "***********FFT READY TO BE PLOTTED*************" << endl;
                /* 1ST HALF*/
                Complex *array_fft = (Complex*)malloc(array_size);
                Complex *ptr = array_fft;
                /*for(unsigned int i = array_size/2;i < array_size;i++)
                {
                    if(array_output[i]->re < 0) array_output[i]->re *=-1;
                    *ptr = *array_output[i];
                    ptr++;
                }
                for(unsigned int i = 1;i <= (array_size/2);i++)
                {
                    if(array_output[i]->re < 0) array_output[i]->re *=-1;
                    *ptr = *array_output[i];
                    ptr++;
                }
                for(unsigned int i = 0;i < array_size;i++)
                {
                    fln = (float)((float)(200)/(float)(array_size));
                    fln *=i;
                    cout << fln << "\t" <<array_fft[i].re <<  endl;
                }*/

                for(unsigned int i = 0;i < array_size/2;i++)
                {
                    if(array_output[i]->re < 0) array_output[i]->re *=-1;
                    fln = (float)((float)(FREQ_SAMPLE)/(float)(array_size));
                    fln *=i;
                    cout << fln/*i*/<< "\t" <<array_output[i]->re <<  endl;
                }

                free(array_fft);
            } // else fft
        //} // while input continues
    }
};

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    fft_processor fft;
    fft.process(argc,argv);

    return 0;
}

