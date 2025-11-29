#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QRandomGenerator>
#include <QPainter>
#include <QPicture>
#include <QFile>
#include <QByteArray>

#define GRID_SIZE_X 10
#define GRID_SIZE_Y 12
#define GRADIENT_LIMIT 3

typedef struct {
    uint8_t direction;
    uint8_t distance;
} grid_command_t;

const grid_command_t Horizontal_Gradients_Cmds_50 [1][25] = {
    {{6,19},{9,1},{8,2},{7,1},{8,1},{7,2},{8,1},{7,1},{8,1},{9,2},{6,1},{9,1},{6,6},{3,1},{6,1},{3,2},{2,3},{1,2},{2,4},{3,1},{6,5},{9,1},{6,14}}
};

//storing horizontal gradients as bitmaps 50x20, 15 above the mid-line, 4 below
const int Horizontal_Gradient_26[260] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,
    2,2,2,2,1,1,1,1,1,1,2,0,0,0,0,2,1,1,1,1,1,1,2,2,2,2,
    1,1,1,1,1,1,1,1,1,1,2,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,2,0,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,2,0,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,2,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static unsigned char Vertical_Gradient_20[200] = {
    0,0,2,1,1,1,1,1,1,1,
    0,0,2,1,1,1,1,1,1,1,
    0,0,2,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,2,2,2,1,1,1,
    0,0,2,2,0,0,0,2,1,1,
    0,0,0,0,0,0,0,0,2,1,
    0,0,0,0,0,0,0,0,2,1,
    0,0,0,0,0,0,0,0,2,1,
    0,0,2,2,0,0,0,2,1,1,
    0,2,1,1,2,2,2,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,0,2,1,1,1,1,1,1,1,
    0,0,2,1,1,1,1,1,1,1,
    0,0,2,1,1,1,1,1,1,1
};

void MainWindow::on_pushButton_generate_grid_clicked()
{
    QFile tga_file("D:\\Saturn\\Re_Joindre\\assets\\test.tga");
    tga_file.open(QIODevice::ReadOnly);
    QByteArray tga = tga_file.readAll();
    tga_file.close();

    //gradients for right and bottom sides
    int Gradients_X[GRID_SIZE_Y - 1][GRID_SIZE_X - 1];
    int Gradients_Y[GRID_SIZE_Y - 1][GRID_SIZE_X - 1];
    //filling the gradient table
    //Gradients_X[0][0] = QRandomGenerator::global()->bounded(-GRADIENT_LIMIT,GRADIENT_LIMIT);
    //Gradients_Y[0][0] = QRandomGenerator::global()->bounded(-GRADIENT_LIMIT,GRADIENT_LIMIT);
    Gradients_X[0][0] = QRandomGenerator::global()->generate();//bounded(0,1);
    Gradients_Y[0][0] = QRandomGenerator::global()->generate();//bounded(0,1);
    for (int y=0;y<(GRID_SIZE_Y-1); y++) {
        for (int x=0;x<(GRID_SIZE_X-1); x++) {
            Gradients_X[y][x] = QRandomGenerator::global()->generate();//bounded(0,1);
            /*if (x==0) {
                Gradients_X[y][x] = QRandomGenerator::global()->bounded(-GRADIENT_LIMIT,GRADIENT_LIMIT);
            }
            else {
                Gradients_X[y][x] = Gradients_X[y][x-1]  + QRandomGenerator::global()->bounded(-2,2);
                Gradients_X[y][x] = (Gradients_X[y][x-1] > GRADIENT_LIMIT) ? GRADIENT_LIMIT : Gradients_X[y][x];
                Gradients_X[y][x] = (Gradients_X[y][x-1] < -GRADIENT_LIMIT) ? -GRADIENT_LIMIT : Gradients_X[y][x];
            }
            if (y==0) {
                Gradients_X[y][x] = QRandomGenerator::global()->bounded(-GRADIENT_LIMIT,GRADIENT_LIMIT);
            }
            else {
                Gradients_X[y][x] = Gradients_X[y-1][x]  + QRandomGenerator::global()->bounded(-2,2);
                Gradients_X[y][x] = (Gradients_X[y-1][x] > GRADIENT_LIMIT) ? GRADIENT_LIMIT : Gradients_X[y][x];
                Gradients_X[y][x] = (Gradients_X[y-1][x] < -GRADIENT_LIMIT) ? -GRADIENT_LIMIT : Gradients_X[y][x];
            }*/
        }
    }

    QPicture pic;
    QPainter pai;
    pai.begin(&pic);
    pai.setPen(Qt::black);
    pai.fillRect(0,0,600,600,QBrush(Qt::yellow));

    //reducing tga resolution with 2x2 average matrix
    QByteArray tga_half;
    for (int y=0;y<240;y++)
        for (int x=0;x<256;x++)
        {
            tga_half.append(((unsigned char)tga[18+(y*1024+x*2)*3] + (unsigned char)tga[18+(y*1024+(x+1)*2)*3] + (unsigned char)tga[18+((y+1)*1024+x*2)*3]  + (unsigned char)tga[18+((y+1)*1024+(x+1)*2)*3])/4);
            tga_half.append(((unsigned char)tga[18+(y*1024+x*2)*3+1] + (unsigned char)tga[18+(y*1024+(x+1)*2)*3+1] + (unsigned char)tga[18+((y+1)*1024+x*2)*3+1]  + (unsigned char)tga[18+((y+1)*1024+(x+1)*2)*3+1])/4);
            tga_half.append(((unsigned char)tga[18+(y*1024+x*2)*3+2] + (unsigned char)tga[18+(y*1024+(x+1)*2)*3+2] + (unsigned char)tga[18+((y+1)*1024+x*2)*3+2]  + (unsigned char)tga[18+((y+1)*1024+(x+1)*2)*3+2])/4);
    }

    uint8_t drawarea[50*50];
    //drawing all tiles
    for (int y=0;y<(GRID_SIZE_Y); y++) {
        for (int x=0;x<(GRID_SIZE_X); x++) {
            int _x = (x+1)*50;
            int _y = (y+1)*40;
            //folowing the cmdlist
            /*pai.drawPoint(_x,_y);
            for (int cmds = 0; cmds < 25; cmds++)
            {
                for (int i=0;i<Horizontal_Gradients_Cmds_50[0][cmds].distance;i++) {
                    switch (Horizontal_Gradients_Cmds_50[0][cmds].direction)
                    {
                    case 1: _x--;_y--;break;
                    case 2: _y--;break;
                    case 3: _x++;_y--;break;
                    case 4: _x--;break;
                    case 6: _x++;break;
                    case 7: _x--;_y++;break;
                    case 8: _y++;break;
                    case 9: _x++;_y++;break;
                    }
                    pai.drawPoint(_x,_y);
                }*/

            //tile size is 26 x 20, tile grid is 10x12, resolution is 260x240
            //cleaning draw area
            memset(drawarea,0,50*50);
            //drawing base quad at coords 10,10
            for (int yy=10;yy<30;yy++)
                for (int xx=10;xx<36;xx++)
                    drawarea[yy*50+xx] = 1;//pixel color

            //top gradient, excluding top tiles
            if (y==0) {
                for (int xx=10;xx<36;xx++)
                    drawarea[10*50+xx] = 2;//border color
            } else {
                if (Gradients_X[y][x] % 2 == 0) {
                    for (int yy=7;yy<17;yy++)
                        for (int xx=10;xx<36;xx++)
                            switch (Horizontal_Gradient_26[(yy-7)*26+(xx-10)]) {
                            case 0:
                                drawarea[yy*50+xx] = 0;//transparent
                                break;
                            case 1:
                                drawarea[yy*50+xx] = 1;//pixel color
                                break;
                            case 2:
                                drawarea[yy*50+xx] = 2;//border color
                                break;
                            }
                } else {
                    for (int yy=4;yy<14;yy++)
                        for (int xx=10;xx<36;xx++)
                            switch (Horizontal_Gradient_26[(13-yy)*26+(xx-10)]) {
                            case 0:
                                drawarea[yy*50+xx] = 1;//pixel color
                                break;
                            case 1:
                                drawarea[yy*50+xx] = 0;//transparent
                                break;
                            case 2:
                                drawarea[yy*50+xx] = 2;//border color
                                break;
                            }
                }
            }

            //bottom gradient, excluding bottom tiles
            if (y == (GRID_SIZE_Y-1)) {
                for (int xx=10;xx<36;xx++)
                    drawarea[29*50+xx] = 2;//border color
            } else {
                if (Gradients_X[y+1][x] % 2 == 0) {
                    for (int yy=26;yy<36;yy++)
                        for (int xx=10;xx<36;xx++)
                            switch (Horizontal_Gradient_26[(yy-26)*26+(xx-10)]) {
                            case 0:
                                drawarea[yy*50+xx] = 1;//pixel color
                                break;
                            case 1:
                                drawarea[yy*50+xx] = 0;//transparent
                                break;
                            case 2:
                                drawarea[yy*50+xx] = 2;//border color
                                break;
                            }
                } else {
                    for (int yy=23;yy<33;yy++)
                        for (int xx=10;xx<36;xx++)
                            switch (Horizontal_Gradient_26[(32-yy)*26+(xx-10)]) {
                            case 0:
                                drawarea[yy*50+xx] = 0;//transparent
                                break;
                            case 1:
                                drawarea[yy*50+xx] = 1;//pixel color
                                break;
                            case 2:
                                drawarea[yy*50+xx] = 2;//border color
                                break;
                            }
                }
            }

            //left gradient, excluding left tiles
            if (x==0) {
                for (int yy=10;yy<30;yy++)
                    drawarea[yy*50+10] = 2;//border color
            } else {
                if (Gradients_Y[y][x] % 2 == 0) {
                    for (int yy=11;yy<29;yy++)
                        for (int xx=8;xx<12;xx++)
                            switch (Vertical_Gradient_20[(yy-10)*10+(xx-8)]) {
                            case 0:
                                drawarea[yy*50+xx] = 0;//transparent
                                break;
                            case 1:
                                drawarea[yy*50+xx] = 1;//pixel color
                                break;
                            case 2:
                                drawarea[yy*50+xx] = 2;//border color
                                break;
                            }
                    for (int yy=16;yy<23;yy++)
                        for (int xx=12;xx<18;xx++)
                            switch (Vertical_Gradient_20[(yy-10)*10+(xx-8)]) {
                            case 0:
                                drawarea[yy*50+xx] = 0;//transparent
                                break;
                            case 1:
                                drawarea[yy*50+xx] = 1;//pixel color
                                break;
                            case 2:
                                drawarea[yy*50+xx] = 2;//border color
                                break;
                            }
                } else {
                    for (int yy=11;yy<29;yy++)
                        for (int xx=3;xx<13;xx++)
                            switch (Vertical_Gradient_20[(yy-10)*10+(12-xx)]) {
                            case 0:
                                drawarea[yy*50+xx] = 1;//pixel color
                                break;
                            case 1:
                                drawarea[yy*50+xx] = 0;//transparent
                                break;
                            case 2:
                                drawarea[yy*50+xx] = 2;//border color
                                break;
                            }
                }
            }

            //right gradient, excluding right tiles
            if (x==(GRID_SIZE_X-1)) {
                for (int yy=10;yy<30;yy++)
                    drawarea[yy*50+36] = 2;//border color
            } else {
                if (Gradients_Y[y][x+1] % 2 == 0) {
                    for (int yy=11;yy<29;yy++)
                        for (int xx=34;xx<43;xx++)
                            switch (Vertical_Gradient_20[(yy-9)*10+(xx-43)]) {
                            case 0:
                                drawarea[yy*50+xx] = 1;//pixel color
                                break;
                            case 1:
                                drawarea[yy*50+xx] = 0;//transparent
                                break;
                            case 2:
                                drawarea[yy*50+xx] = 2;//border color
                                break;
                            }
                } else {
                    for (int yy=16;yy<25;yy++)
                        for (int xx=28;xx<38;xx++)
                            switch (Vertical_Gradient_20[(yy-9)*10+(27-xx)]) {
                            case 0:
                                drawarea[yy*50+xx] = 0;//pixel color
                                break;
                            case 1:
                                drawarea[yy*50+xx] = 1;//transparent
                                break;
                            case 2:
                                drawarea[yy*50+xx] = 2;//border color
                                break;
                            }
                    for (int yy=11;yy<29;yy++)
                        for (int xx=32;xx<38;xx++)
                            switch (Vertical_Gradient_20[(yy-9)*10+(27-xx)]) {
                            case 0:
                                drawarea[yy*50+xx] = 0;//pixel color
                                break;
                            case 1:
                                drawarea[yy*50+xx] = 1;//transparent
                                break;
                            case 2:
                                drawarea[yy*50+xx] = 2;//border color
                                break;
                            }
                }
            }

            //tga_half.append(tga_half);
            //copy drawarea to test image
            pai.setPen(Qt::black);
            for (int yy=0;yy<50;yy++)
                for (int xx=0;xx<50;xx++)
                    if ( 2 == drawarea[yy*50+xx])
                        pai.drawPoint(_x+xx,_y+yy);
            int index;
            for (int yy=0;yy<50;yy++)
                for (int xx=0;xx<50;xx++)
                    if ( 1 == drawarea[yy*50+xx]) {
                        index = ((y*20+yy-10)*256+x*25+xx-10)*3;
                        if ((index+2) < tga_half.size())
                        {
                            pai.setPen(QColor((unsigned char)tga_half[((y*20+yy-10)*256+x*25+xx-10)*3+2],
                                              (unsigned char)tga_half[((y*20+yy-10)*256+x*25+xx-10)*3+1],
                                              (unsigned char)tga_half[((y*20+yy-10)*256+x*25+xx-10)*3+0],
                                              (int)255));
                            pai.drawPoint(_x+xx,_y+yy);
                        }
                    }
            }
    }

    //test draw tga
    /*for (int y=0;y<480;y++)
        for (int x=0;x<512;x++) {
            pai.setPen(QColor((unsigned char)tga[18+(y*512+x)*3+2],
                              (unsigned char)tga[18+(y*512+x)*3+1],
                              (unsigned char)tga[18+(y*512+x)*3+0],
                              (int)255));
            pai.drawPoint(x+50,y+50);
        }*/

    //test draw tga
    for (int y=0;y<240;y++)
        for (int x=0;x<256;x++) {
            pai.setPen(QColor((unsigned char)tga_half[(y*256+x)*3+2],
                              (unsigned char)tga_half[(y*256+x)*3+1],
                              (unsigned char)tga_half[(y*256+x)*3+0],
                              (int)255));
            pai.drawPoint(x+600,y+50);
        }


    pai.end();

    ui->label_grid->setPicture(pic);
}

