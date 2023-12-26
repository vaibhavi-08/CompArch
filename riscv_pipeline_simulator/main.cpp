#include<bits/stdc++.h>
using namespace std;
map<int,string> instruction_memory;//each string contained here is of length 32(4bytes)
map<string,string> gpr;
map<string,string> DM;
string OP="*******";//add opcode for r type;
string OPIMM="*******";//add opcode for i type;
string BRANCH="*******";//add opcode for branch;
string JALR="*******";//add opcode for jal;
string LOAD="*******";//load opcode;
string STORE="******";//store opcode;
struct controlInfo
{
    bool ALUSrc= false;
    string ALUOp;
    bool branch= false;
    bool mem2Reg= false;
    bool jump= false;
    bool memWrite=false;
    bool memRead=false;
    //string ALUSelect;
    bool regWrite=false;
    bool regRead= false;
};
struct IFID
{
    int dpc;
    string ir;
};
void flush_ifid(IFID* &x){
    x->dpc=0;
    x->ir="";
}
struct IDEX{
    int djpc;
    string immb;
    controlInfo* cw;
    string rs1;
    string rs2;
    string imm;
    string fn3;
    string rdl;
    string alusel;
};
void flush_idex(IDEX* &x){
    x->cw->regRead=false;
    x->cw->ALUSrc= false;
    x->cw->branch= false;
    x->cw->jump=false;
    x->cw->regWrite=false;
    x->cw->ALUOp="";
    x->cw->memWrite=false;
    x->cw->memRead= false;
    x->cw->mem2Reg=false;
    x->djpc=0;
    x->rs2="";
    x->immb="";
    x->alusel="";
    x->imm="";
    x->rs1="";
    x->fn3="";
    x->rdl="";
}
struct EXMO{
    controlInfo* cw;
    string aluout;
    string rs2;
    string rdl;
};
struct MOWB{
    controlInfo* cw;
    string ldout;
    string aluout;
    string rdl;
};
string setAluOp(string &opc){
    if(opc==LOAD||opc==STORE){
        return "00";
    }
    else if(opc==BRANCH||opc==JALR){
        return "01";
    }
    else{
        return "10";
    }
}
string setAluCtrl(string &aluOp,string &fn3,string &fn7){
    if(aluOp=="00"){
        return "0010";
    }
    else if(aluOp=="01"){
        return "0110";
    }
    else{//r type
        if(fn7=="0000000"){
            if(fn3=="000"){//add
                return "0010";
            }
            else if(fn3=="111"){//and
                return "0000";
            }
            else{//or
                return "0001";
            }
        }
        else{//0100000
            return "0110";
        }
    }
}
controlInfo* controller(string &opc){
    controlInfo* x=new controlInfo();
    if(opc==BRANCH){
        x->branch=true;
        x->ALUSrc=true;
        x->regRead=true;
    }
    else if(opc==JALR){
        x->jump=true;
        x->ALUSrc=true;
        x->regRead=true;
    }
    else if(opc==LOAD){
        x->regRead=true;
        x->regWrite=true;
        x->ALUSrc=true;
        x->mem2Reg=true;
        x->memRead=true;
    }
    else if(opc==STORE){
        x->regRead=true;
        x->ALUSrc=true;
        x->memWrite=true;
    }
    else{
        x->regRead=true;
        x->regWrite=true;
    }
    x->ALUOp= setAluOp(opc);
    return x;
}
string intToBinary(int number) {
    // Use std::bitset to convert the integer to binary
    std::bitset<32> binary(number);

    // Convert the binary representation to a string
    return binary.to_string();
}
string itob5(int number){
    std::bitset<5> binary(number);

    // Convert the binary representation to a string
    return binary.to_string();
}
string alu(string &ALUSelect,string &data1,string &data2){
    int r1=stoi(data1,0,2);
    int r2=stoi(data2,0,2);
    int ans;
    if(ALUSelect=="0010")
    {
        ans=r1+r2;
    }
    else if(ALUSelect=="0110"){
        ans=r1-r2;
    }
    else if(ALUSelect=="0000"){
        ans=r1&r2;
    }
    else{
        ans=r1|r2;
    }
    return intToBinary(ans);
}
string forwader(string &rsl,string& rdl1,string &rdl2){
    if(rsl==rdl1){
        return gpr[rdl1];
    }
    else if(rsl==rdl2){
        return gpr[rdl2];
    }
    else{
        return gpr[rsl];
    }
}
string jmp(string imm,string rs1){
    int p=stoi(imm,0,2);
    int q=stoi(gpr[rs1]);
    return intToBinary(p+q);
}
int bac(string imm,int pc){
    //assembling the address
    string imm12="";
    imm12+=imm[0];
    imm12+=imm[7];
    imm12=imm12+imm.substr(1,6);
    imm12=imm12+imm.substr(8,4);
    int ofs=stoi(imm12,0,2);
    ofs=ofs<<1;
    return pc+ofs;
}




int main(){
    //creating registers;
    for(int i=0;i<32;i++){
        gpr[itob5(i)]= intToBinary(0);
    }
    //push instructions from file to instruction memory
    string filename="example.txt";//enter your filename in which instructions are stored as machine code
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return 1; // Return an error code
    }
    string line;
    int add=0;
    while(getline(inputFile,line)){
        instruction_memory[add]=line;
        add=add+4;
    }
    inputFile.close();
    //instructions loaded into instruction memory

    IFID* ifid=new IFID();
    IDEX* idex=new IDEX();
    EXMO* exmo=new EXMO();
    MOWB* mowb=new MOWB();
    int pc=0;
    long long M=instruction_memory.size();
    while(pc!=M){
        //write-back
        if(mowb->cw->regWrite){
            if(mowb->cw->mem2Reg){
                gpr[mowb->rdl]=mowb->ldout;
            }
            else{
                gpr[mowb->rdl]=mowb->aluout;
            }
        }
        //memory-operation
        if(exmo->cw->memWrite){
            DM[exmo->aluout]=gpr[exmo->rs2];
        }
        if(exmo->cw->memRead){
            mowb->ldout=DM[exmo->aluout];
        }
        mowb->cw=exmo->cw;
        mowb->aluout=exmo->aluout;
        mowb->rdl=exmo->rdl;
        //execute
        string inp1= forwader(idex->rs1,exmo->rdl,mowb->rdl);
        string inp2;
        if(idex->rs2.empty()){
            inp2=idex->imm;
        }
        else{
            inp2= forwader(idex->rs2,exmo->rdl,mowb->rdl);
        }
        exmo->aluout=alu(idex->alusel,inp1,inp2);
        bool flag=(inp1==inp2);
        exmo->cw=idex->cw;
        exmo->rdl=idex->rdl;
        exmo->rs2=idex->rs2;
        if(idex->cw->branch&&flag){
            pc=bac(idex->immb,idex->djpc);
            //flush logic
            flush_idex(idex);
            flush_ifid(ifid);
            continue;
        }
        if(idex->cw->jump){
            pc=idex->djpc;
            flush_idex(idex);
            flush_ifid(ifid);
            continue;
        }
        //decode
        string opcode=ifid->ir.substr(25,7);
        string rs1=ifid->ir.substr(12,5);
        string imm=ifid->ir.substr(0,12);
        string fn3=ifid->ir.substr(17,3);
        string rdl=ifid->ir.substr(20,5);
        string fn7=ifid->ir.substr(0,7);
        string bimm=fn7+rdl;
        idex->imm=imm;
        idex->fn3=fn3;
        idex->rdl=rdl;
        idex->alusel= setAluCtrl(opcode,fn3,fn7);
        idex->immb=bimm;
        idex->cw=controller(opcode);
        if(idex->cw->jump){
            idex->djpc= stoi(jmp(imm,rs1),0,2);
        }
        else{
            idex->djpc=ifid->dpc;
        }
        if(idex->cw->regRead){
            idex->rs1=rs1;
            if(!idex->cw->ALUSrc){
                idex->rs2=ifid->ir.substr(7,5);
            }
        }
        //fetch
        ifid->ir=instruction_memory[pc];
        ifid->dpc=pc;
        pc=pc+4;
    }

    return 0;
}

