#include <windows.h>
#include <time.h>
#include <process.h>
#include <iostream>
#include <cstdio>
#include <string.h>
#include "parser.h"
#include "util.h" 
#pragma comment(linker, "/subsystem:console /entry:WinMainCRTStartup")
 
using namespace std;
const char g_szClassName[] = "myMpegDemoClass";
int frame_cnt = 10; // prepare some loading time for opening and decoding
extern PIC_BUF pic_buf[200];
void run_background(void* argv) {
    printf("%s\n", (char*)argv);
    if(decode_init((char*)argv, 0)) decode_video_sequence();
    cout << "decode process end" << endl;
}
 
// Ref: http://stackoverflow.com/questions/431470/window-border-width-and-height-in-win32-how-do-i-get-it
void ClientResize(HWND hWnd, int nWidth, int nHeight)
{
    RECT rcClient, rcWind;
    POINT ptDiff;
    GetClientRect(hWnd, &rcClient);
    GetWindowRect(hWnd, &rcWind);
    ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
    ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;
    MoveWindow(hWnd,rcWind.left, rcWind.top, nWidth + ptDiff.x, nHeight + ptDiff.y, TRUE);
}
 
void frame_update(HWND hwnd, double start_time) {
     
    // Wait until next frame should be shown  
    while((clock()-start_time)/CLOCKS_PER_SEC < 1/pictures_per_second[picture_rate]);
    // Next frame
    frame_cnt++;
    int horizontal, vertical;
    get_hor_ver(&horizontal, &vertical);
    int width = horizontal;//pic_buf[0].horizontal_size;
    int height = vertical;//pic_buf[0].vertical_size;
    HDC hdc = GetDC(hwnd);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbm = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);
    int *buffer = new int[width*height];
     
    // Lock window's size to fit the clip
    ClientResize(hwnd, width, height);
     
    // Retrieve next frame
    static int frame_num = 0;
    for(int m = 0; m < height; m++) {
        for(int n = 0; n < width; n++) {
            double dy = pic_buf[frame_num].ycbcr[0][m][n]-16.0;
            double dcb = pic_buf[frame_num].ycbcr[1][m>>1][n>>1]-128.0;
            double dcr = pic_buf[frame_num].ycbcr[2][m>>1][n>>1]-128.0;
            int R = 0.5 + 255.0/219*dy + 255.0/112*0.701*dcr;
            int G = 0.5 + 255.0/219*dy - 255.0/112*0.886*0.114/0.587*dcb - 255.0/112*0.701*0.299/0.587*dcr;
            int B = 0.5 + 255.0/219*dy + 255.0/112*0.886*dcb;
            R = R>255?255:R<0?0:R;
            G = G>255?255:G<0?0:G;
            B = B>255?255:B<0?0:B;
            buffer[m * width + n] = (R<<16)|(G<<8)|B;
        }
    }
    // And set to HBITMAP
    SetBitmapBits(hbm, width * height * sizeof(int), buffer);
     
    // Window's size is fixed, so just use BitBlt
    BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
    frame_num++;
    cout << frame_num<<endl; 

     
    // Output
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbm);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdc);
    delete [] buffer;
     
}
 
// Step 4. the Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CLOSE: // X button
            DestroyWindow(hwnd); // destroy the specific window
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Test thread
    // See http://stackoverflow.com/questions/5968076/passing-parameters-to-beginthreadex
    int argc = 0; 
    LPWSTR *argv; 
    argv = CommandLineToArgvW ((LPCWSTR )lpCmdLine, &argc); 
    _beginthread(run_background, 0, (char *)argv[0]);
     
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;
     
    // Step 1. Register the window class
    // See http://www.winprog.org/tutorial/simple_window.html
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
     
    // Step 2. Create the Window
    hwnd = CreateWindowEx (
        WS_EX_CLIENTEDGE, // extended window style, can try 0 or more other values
        g_szClassName,
        "MPEG decoder",
        WS_OVERLAPPEDWINDOW, // window style parameter
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 240, // X and Y co-ordinate of left top, width, height
        NULL, NULL, hInstance, NULL // parent window handle, menu handle, application instance handle, pointer to window creation data
    );
    if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    ShowWindow(hwnd, nCmdShow); // show the window
    UpdateWindow(hwnd); // redraw
     
     
    // Modified Step 3.   
    Sleep(1000);
    while(1) {
       
        if(PeekMessage(&Msg, NULL, 0, 0, 0)) {  // non-blocking
            if(GetMessage(&Msg, NULL, 0, 0) > 0) {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
            else break;
        }
        //cout << "frame_update" << endl;
        double start_time = clock();
        frame_update(hwnd, start_time);
    }
    return Msg.wParam;
}