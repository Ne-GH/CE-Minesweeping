#include <iostream>
#include <conio.h>
#include <windows.h>

using namespace std;


class Pos {
public:
    int _x;
    int _y;
    Pos() {
        _x = 0;
        _y = 0;
    }
    Pos(int x, int y) :_x(x), _y(y) {}
};

// 简单封装下Windows API
void SetMousePos(Pos pos) {
    SetCursorPos(pos._x, pos._y);
}
void MouseLeftEvent() {
    mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}
void MouseRightEvent() {
    mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
}
template<typename T>
void GetArr(T **&arr,int h,int w) {
    arr = new T *[h];

    for (int i = 0; i < h; ++i) {
        arr[i] = new T[w];
    }

}
template<typename T>
void DelArr(T **&arr,int h,int w) {
    for (int i = 0; i < h; ++i) {
        delete[]arr[i];
    }
    delete arr;
    arr = nullptr;
}


HWND handle;
DWORD pid;
HANDLE process_handle;
RECT rect;
BYTE h, w;
BYTE* buf;
int use_time;
bool OpenProcess() {
    // 打开扫雷进程
    handle = FindWindow(
        NULL,
        "Minesweeper"
    );
    if (handle == NULL) {
        cout << "打开失败" << endl;
        return false;
    }
    return true;
}

void GetHandle() {
    GetWindowThreadProcessId(handle, &pid);
    process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
}
void GetMinersArr(){
    ReadProcessMemory(process_handle, (void*)0x01005338, &h, sizeof(h), NULL);
    ReadProcessMemory(process_handle, (void*)0x01005334, &w, sizeof(h), NULL);
    buf = (BYTE*)malloc(h * 32);
    ReadProcessMemory(process_handle, (void*)0x01005361, (void*)buf, h * 32, NULL);
}
int main() {
    // 修改cmd显示编码,防止中文乱码
    system("chcp 65001");
   
    
    OpenProcess();
    GetHandle();

    while (1) {
        cout << "输入此次扫雷用时(s);";
        cin >> use_time;
        use_time--;
        GetMinersArr();


        // 获取窗口位置,并依此移动鼠标

        GetWindowRect(handle, &rect);
        cout << "窗口位置:" << rect.left << "," << rect.top;
        cout << "行数:" << (int)h << endl;
        cout << "列数:" << (int)w << endl;
        SetMousePos(Pos(rect.left, rect.top));
        MouseLeftEvent();

        // 将鼠标移动到扫雷的左上第一个点
        POINT begin_pos;
        begin_pos.x = rect.left + 20;
        begin_pos.y = rect.top + 105;
        SetMousePos(Pos(begin_pos.x, begin_pos.y));
        MouseLeftEvent();

        Pos** mouse_pos;
        bool** mines;
        GetArr<Pos>(mouse_pos, h, w);
        GetArr<bool>(mines, h, w);

        // 将每个点的鼠标坐标都计算出来,并保存到数组中
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                mouse_pos[i][j] = Pos(begin_pos.x + 16 * j, begin_pos.y + 16 * i);
            }
        }

        int num = 0;
        int i, j;
        BYTE* line = buf;
        for (i = 0; i < h; i++) {
            for (j = 0; j < w; j++) {
                if (line[j] == 0x8F) {
                    mines[i][j] = true;
                    num++;
                }
                printf("%2X ", line[j]);
            }
            cout << endl;
            line = line + 32;
        }
        cout << num << endl;
        num = 0;
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                if (mines[i][j] == true)
                    num++;
                printf("%2d", mines[i][j]);
            }
            cout << endl;
        }
        cout << num << endl;
        num = 0;
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                SetMousePos(mouse_pos[i][j]);
                // 如果当前鼠标下是雷,模拟鼠标右键
                if (mines[i][j] == true) {
                    MouseRightEvent();
                    num++;
                }
                // 否则模拟左键
                else {

                    MouseLeftEvent();
                }
                
            }
            WriteProcessMemory(process_handle, (void*)0x0100579C, &use_time, sizeof(use_time), NULL);
        }
        cout << num << endl;

        DelArr(mines, h, w);
        DelArr(mouse_pos, h, w);
     
        WriteProcessMemory(process_handle, (void*)0x0100579C, &use_time, sizeof(use_time), NULL);


    }



   



    return 0;
}