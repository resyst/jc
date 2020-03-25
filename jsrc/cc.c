/* Copyright 1990-2010, Jsoftware Inc.  All rights reserved.               */
/* Licensed use only. Any other use is in violation of copyright.          */
/*                                                                         */
/* Conjunctions: Cuts                                                      */

#include "j.h"
#include "vcomp.h"
#define ZZDEFN
#include "result.h"
#define STATENEEDSASSEMBLYX 11
#define STATENEEDSASSEMBLY (((I)1)<<STATENEEDSASSEMBLYX)



static DF1(jtcut01){DECLF;A h,x,z;
 RZ(x=from(box(every(negate(shape(w)),0L,jtiota)),w));
 if(VGERL&sv->flag){h=sv->fgh[2]; R df1(z,x,*AAV(h));}else R CALL1(f1,x,fs);
}    /* f;.0 w */

static DF2(jtcut02){F2PREFIP;A fs,q,qq,*qv,z,zz=0;I*as,c,e,i,ii,j,k,m,n,*u,*ws;PROLOG(876);I cger[128/SZI];
 RZ(a&&w);
#define ZZFLAGWORD state
 I state=0;  // init flags, including zz flags


 if(!(VGERL&FAV(self)->flag)){
  fs=FAV(self)->fgh[0];  // the verb we will execute
 }else{
  RZ(fs=createcycliciterator((A)&cger, self));  // use a verb that cycles through the gerunds.
 }
 if(FAV(fs)->mr>=AR(w)){
  // we are going to execute f without any lower rank loop.  Thus we can use the BOXATOP etc flags here.  These flags are used only if we go through the full assemble path
  state = (FAV(fs)->flag2&VF2BOXATOP1)>>(VF2BOXATOP1X-ZZFLAGBOXATOPX);  // Don't touch fs yet, since we might not loop
  state &= ~((FAV(fs)->flag2&VF2ATOPOPEN1)>>(VF2ATOPOPEN1X-ZZFLAGBOXATOPX));  // We don't handle &.> here; ignore it
  state |= (-state) & (I)jtinplace & (ZZFLAGWILLBEOPENED|ZZFLAGCOUNTITEMS); // remember if this verb is followed by > or ; - only if we BOXATOP, to avoid invalid flag setting at assembly
 }
 AF f1=FAV(fs)->valencefns[0];  // point to the action routine now that we have handled gerunds

 I wr=AR(w);  // rank of w
 RZ(a=vib(a));  // audit for valid integers
 if(1>=AR(a))RZ(a=lamin2(zeroionei(0),a));   // default list to be lengths starting at origin
 as=AS(a); m=AR(a)-2; PROD(n,m,as); c=as[1+m]; u=AV(a);  // n = # 2-cells in a, c = #axes of subarray given, m is index of next-last axis of a, u->1st atom of a
 ASSERT((-(as[m]^2)|(wr-c))>=0,EVLENGTH);    // shapes must end with 2,c where c does not exceed rank of r
 if(!n){  /* empty result; figure out result type */
  z=CALL1(f1,w,fs);
  if(z==0)z=zeroionei(0);  // use zero as fill result if error
  GA(zz,AT(z),n,m+AR(z),0); I *zzs=AS(zz); I *zs=AS(z); 
  MCISH(zzs,as,m) MCISH(zzs+m,zs,AR(z)) // move in frame of a followed by shape of result-cell
  RETF(zz);
 }
 // otherwise general case, one axis at a time
 ws=AS(w);
#define ZZDECL
#include "result.h"
 // If there is only one column, we will be using  a virtual block to access the subarray, so allocate it here
 //  (if the subarray is reversed, we won't need the block, but that's rare)
 I wcellsize;  // size of a cell in atoms of w.
 I origoffset;  // offset from virtual block to start of w
 I wcellbytes;  // size of a cell in bytes
 A virtw;  // virtual block to use if any
 fauxblock(virtwfaux);
 if(c==1){
  PROD1(wcellsize,wr-1,ws+1);  // size in atoms of w cell
  // allocate virtual block
  fauxvirtual(virtw,virtwfaux,w,wr,ACUC1);  // allocate UNINCORPORABLE block, noninplaceable
  // fill in shape
  MCISH(AS(virtw)+1,ws+1,wr-1);
  // remember original offset.  Others will be based on this
  origoffset=AK(virtw);
  wcellbytes=wcellsize<<bplg(AT(w));  // bytes per cell
 }
 if(m){
   // There is a frame; we will have to assemble the results, even if there is only one
   // See if this verb is BOXATOP.  NOTE that if this is a gerund, fs points to a cyclic iterator which is never BOXATOP
   // We honor BOXATOP if the verb can operate on a cell of w in its entirety
   state |= STATENEEDSASSEMBLY;  // force us to go through the assembly code
   ZZFLAGWORD |= ((FAV(fs)->mr>=wr?VF2BOXATOP1:0)&(FAV(fs)->flag2))>>(VF2BOXATOP1X-ZZFLAGBOXATOPX);  // If this is BOXATOP, set so for loop.
   ZZPARMS(m,n,1)
#define ZZINSTALLFRAME(optr) MCISHd(optr,as,m)
 }
 I gerundx=0; q=0;  // initialize to first gerund; we haven't allocated the input to {
 for(ii=n;ii;--ii){  // for each 2-cell of a.  u points to the cell
  if((-(c^1)|(e=u[1]))>=0){I axislen;  // e=length of 1st axis
   // non-reversed selection along a single axis: calculate length {. start }. w
   axislen=ws[0]; j=u[0];  // axislen=length of axis, j=starting position
   if(j>=0){e=MIN(e,axislen-j);}  // truncate length to length remaining; if <0, error
   else{j+=axislen; e=MIN(e,j+1);  j-=(e-1);}  // negative j: convert to positive index of ending item; truncate length to length remaining; back j to first item.  Error if j<0 and e!=0, or j<_1
   ASSERT(e>=0,EVINDEX);
   // Allocate a virtual block & fill it in
   // locate virtual block
   AK(virtw)=origoffset+j*wcellbytes;
   // fill in shape.  Rest of shape filled in at allocation
   AS(virtw)[0]=e; // shape of virtual matches shape of w except for #items
   AN(virtw)=e*wcellsize;  // install # atoms
   z = virtw;  // use the virtual block for the computation
  }else{I axislen;
   // general selection: multiple axes, or reversal.  We do not look for the case of one reversed axis (could avoid boxing input to {) on grounds of rarity
   do{
    if(q){  // if we have already allocated the input area to {, fill it in
     // For each axis, create a boxed vector of boxed indexes to fetch, in the correct order
     for(i=0;i<c;++i){  // for each axis of the cell of a
      axislen=ws[i]; j=u[i]; e=u[i+c]; k=e; e=REPSGN(e); k^=e; k-=e;  // axislen=length of this axis, j=starting pos, e=sgn(length), k=ABS(length)
      if(j>=0){k=MIN(k,axislen-j); j+=e&(k-1);}else{j+=axislen; k=MIN(k,j+1); j-=(~e)&(k-1);}  // adjust j for negative j; clip endpoint to length of axis; move j to end of interval if reversed
      ASSERT(k>=0,EVINDEX); RZ(qv[i]=apv(k,j,2*e+1));  // create ascending or descending vector.  The increment is 1 or -1
     }
     break;  // this is the loop exit
    }else{  // we have not allocated the input to {; do so now
     GATV0(q,BOX,c,1); qv=AAV(q);   // allocate a vector of boxes, which will contain the selectors
     GAT0(qq,BOX,1,0); *AAV(qq)=q;  // enclose that vector of boxes in a box to pass into {
    }
   }while(1);
   RZ(z=from(qq,w));
  }
  RZ(z=CALL1(f1,z,fs));
  if(!(state&STATENEEDSASSEMBLY)){if(AFLAG(z)&AFUNINCORPABLE){z=clonevirtual(z);} EPILOG(z);}  // if we have just 1 input and no frame, return the one result directly (for speed).  If it is UNINCORPABLE, it must not be allowed to escape - realize it
      // we use clonevirtual so that ];.0 can return a virtual block
#define ZZBODY
#include "result.h"

  u += 2*c;  // step to the next cell of a
 }
#define ZZEXIT
#include "result.h"
 EPILOG(zz);
}    /* a f;.0 w */

// self is a compound, using @/@:/&/&:, that we tried to run with special code, but we found that we don't support the arguments
// here we revert to the non-special code for the compound
DF2(jtspecialatoprestart){
  RZ(a&&w&&self);  // return fast if there has been an error
  V *sv=FAV(self);  // point to verb info for the current overall compound
  R a==mark?(sv->id==CFORK?jtcork1:on1)(jt,w,self) : (sv->id==CFORK?jtcork2:jtupon2)(jt,a,w,self);  // figure out the default routine that should process the compound, and transfer to it
}


// a ;@:(<;.0) w where a has one column.  We allocate a single area and copy in the items
// if we encounter a reversal, we abort
// if it's a case we can't handle, we fail over to the normal code, with BOXATOP etc flags set
DF2(jtrazecut0){A z;C*wv,*zv;I ar,*as,(*av)[2],j,k,m,n,wt;
 RZ(a&&w);
 wt=AT(w); wv=CAV(w);
 ar=AR(a); as=AS(a);
 // we need rank of a>2 (otherwise why bother?), rank of w>0, w not sparse, a not empty, w not empty (to make item-size easier)
 if((((ar-3)|(-(wt&SPARSE))|(AR(w)-1)|(AN(a)-1)))<0)R jtspecialatoprestart(jt,a,w,self);
 // the 1-cells of a must be 2x1
 if((as[ar-2]^2)|(as[ar-1]^1))R jtspecialatoprestart(jt,a,w,self);
 n=AS(w)[0]; m=AN(a)>>1;  // number of items of w, number of w-items in result
 // pass through a, counting result items.  abort if there is a reversal - too rare to bother with in the fast path
 RZ(a=vib(a)); av=(I(*)[2])AV(a); I nitems=0;
 // verify starting value in range (unless length=0); truncate length if overrun
 DO(m, j=av[i][0]; k=av[i][1]; if(k<0)R jtspecialatoprestart(jt,a,w,self); I jj=j+n; jj=(j>=0)?j:jj; ASSERT(((jj^(jj-n))|(k-1))<0,EVINDEX); j=n-jj; k=k>j?j:k; nitems+=k; ASSERT(nitems>=0,EVLIMIT))
 // audits passed and we have counted the items.  Allocate the result and copy
 I wcn; PROD(wcn,AR(w)-1,AS(w)+1);  // number of atoms per cell of w
 I zn; RE(zn=mult(wcn,nitems));  // number of atoms in result
 GA(z,wt,zn,AR(w),AS(w)); AS(z)[0]=nitems; zv=CAV(z);  // allocate a list of items of w, fill in length.  zv is running output pointer
 // copy em in.  We use MC because the strings are probably long and unpredictable - think HTML parsing
 I wcb=wcn<<bplg(wt);  // number of bytes in a cell of w
 DO(m, j=av[i][0]; k=av[i][1]; I jj=j+n; jj=(j>=0)?j:jj; j=n-jj; k=k>j?j:k; k*=wcb; MC(zv,wv+jj*wcb,k); zv+=k;)
 RETF(z);
}    /* a ;@:(<;.0) vector */


static DF2(jtcut2bx){A*av,b,t,x,*xv,y,*yv;B*bv;I an,bn,i,j,m,p,q,*u,*v,*ws;
 RZ(a&&w&&self);
 q=(I)FAV(self)->localuse.lvp[0];  // fetch the n in the original u;.n
 an=AN(a); av=AAV(a);  ws=AS(w);
 ASSERT(an<=AR(w),EVLENGTH);
 GATV0(x,BOX,an,1); xv=AAV(x);  // could be faux
 GATV0(y,BOX,an,1); yv=AAV(y);
 for(i=0;i<an;++i){
  b=av[i]; bn=AN(b); m=ws[i];
  ASSERT(1>=AR(b),EVRANK);
  if(!bn&&m){xv[i]=num(0); RZ(yv[i]=sc(m));}
  else{
   if(!(B01&AT(b)))RZ(b=cvt(B01,b));
   if(!AR(b)){if(*BAV(b)){RZ(xv[i]=IX(m)); RZ(yv[i]=reshape(sc(m),num(0<q)));}else xv[i]=yv[i]=mtv; continue;}
   ASSERT(bn==m,EVLENGTH);
   bv=BAV(b); p=0; DO(bn, p+=bv[i];); 
   GATV0(t,INT,p,1); u=AV(t); xv[i]=t;
   GATV0(t,INT,p,1); v=AV(t); yv[i]=t; j=-1;
   if(p)switch(q){
    case  1: DO(bn, if(bv[i]){*u++=i  ; if(0<=j)*v++=i-j  ; j=i;}); *v=bn-j;   break;
    case -1: DO(bn, if(bv[i]){*u++=i+1; if(0<=j)*v++=i-j-1; j=i;}); *v=bn-j-1; break;
    case  2: DO(bn, if(bv[i]){*u++=j+1;         *v++=i-j  ; j=i;}); break;
    case -2: DO(bn, if(bv[i]){*u++=j+1;         *v++=i-j-1; j=i;}); break;
   }
  }
 }
 RZ(x=ope(catalog(x)));
 RZ(y=ope(catalog(y)));
 if(AN(x)){RZ(IRS2(x,y,0L,1L,1L,jtlamin2,t));}else{RZ(t=iota(over(shape(x),v2(2L,0L))));}
 R cut02(t,w,self);
}    /* a f;.n w for boxed a, with special code for matrix w */


#define CUTSWITCH(EACHC)  \
 switch(id){A z,*za;C id1,*v1,*zc;I d,i,j,ke,q,*zi,*zs;                 \
  case CPOUND:                                                               \
   GATV0(z,INT,m,1); zi=AV(z); EACHC(*zi++=d;); R z;                          \
  case CDOLLAR:                                                              \
   GATV0(z,INT,m,1); zi=AV(z); EACHC(*zi++=d;);                               \
   A zz,zw; RZ(zw=vec(INT,MAX(0,r-1),1+s)); IRS2(z,zw,0L,0L,1L,jtover,zz); RETF(zz);  \
  case CHEAD:                                                                \
   GA(z,t,m*c,r,s); zc=CAV(z); AS(z)[0]=m;                                     \
   EACHC(ASSERT(d,EVINDEX); MC(zc,v1,k); zc+=k;);                            \
   R z;                                                                      \
  case CTAIL:                                                                \
   GA(z,t,m*c,r,s); zc=CAV(z); AS(z)[0]=m;                                     \
   EACHC(ASSERT(d,EVINDEX); MC(zc,v1+k*(d-1),k); zc+=k;);                    \
   R z;                                                                      \
  case CCOMMA:                                                               \
  case CLEFT:                                                                \
  case CRIGHT:                                                               \
   e-=e&&neg; RE(d=mult(m*c,e));                                             \
   GA(z,t,d,id==CCOMMA?2:1+r,s-1); zc=CAV(z); fillv(t,d,zc);                 \
   zs=AS(z); zs[0]=m; zs[1]=id==CCOMMA?e*c:e; ke=k*e;                        \
   EACHC(MC(zc,v1,d*k);  zc+=ke;);                                           \
   R z;                                                                      \
  case CBOX:                                                                 \
   GA(z,m?BOX:B01,m,1,0); za=AAV(z);                                         \
   EACHC(GA(y,t,d*c,r,s); AS(y)[0]=d; MC(AV(y),v1,d*k); *za++=y;);             \
   R z;                                                                      \
  case CAT: case CATCO: case CAMP: case CAMPCO:                              \
   if(CBOX==ID(vf->fgh[0])&&(id1=ID(vf->fgh[1]),id1==CBEHEAD||id1==CCTAIL)){           \
    GA(z,m?BOX:B01,m,1,0); za=AAV(z);                                        \
    EACHC(d=d?d-1:0; GA(y,t,d*c,r,s); AS(y)[0]=d; MC(AV(y),id1==CBEHEAD?v1+k:v1,d*k); *za++=y;);               \
    R z;                                                                     \
   }                                                                         \
   /* note: fall through */                                                  \
  default:                                                                   \
   if(!m){y=reitem(zeroionei(0),w); R iota(over(zeroionei(0),shape(h?df1(z,y,*hv):CALL1(f1,y,fs))));}                            \
   GATV0(z,BOX,m,1); za=AAV(z); j=0;                                          \
   if(h){EACHC(GA(y,t,d*c,r,s); AS(y)[0]=d; MC(AV(y),v1,d*k); A Zz; RZ (df1(Zz,y,hv[j])); j=(1+j)%hn; rifv(Zz); *za++=Zz;); \
   }else{EACHC(GA(y,t,d*c,r,s); AS(y)[0]=d; MC(AV(y),v1,d*k); A Zz; RZ(Zz = CALL1(f1,y,fs)); rifv(Zz); *za++=Zz; ); \
   }                                                                         \
   z=ope(z);                                                                 \
   EPILOG(z);                                                                \
 }

#define EACHCUTSP(stmt)  \
 if(pfx)for(i=m;i>=1;--i){q=yu[i-1]-yu[i  ]; d=q-neg; v1=wv+k*(b+p); stmt; p+=q;}  \
 else   for(i=1;i<=m;++i){q=yu[i  ]-yu[i-1]; d=q-neg; v1=wv+k*(b+p); stmt; p+=q;}

static F1(jtcps){A z;P*wp,*zp;
 GASPARSE(z,AT(w),1,AR(w),AS(w)); 
 zp=PAV(z);
 wp=PAV(w); 
 SPB(zp,a,SPA(wp,a)); 
 SPB(zp,e,SPA(wp,e));
 SPB(zp,i,SPA(wp,i));
 R z;
}

static A jtselx(J jt,A x,I r,I i){A z;I c,k;
 c=aii(x); k=c<<bplg(AT(x));
 GA(z,AT(x),r*c,AR(x),AS(x)); AS(z)[0]=r;
 MC(CAV(z),CAV(x)+i*k,r*k);
 R z;
}    /* (i+i.r){x */

static A jtsely(J jt,A y,I r,I i,I j){A z;I c,*s,*v;
 c=*(1+AS(y));
 GATV0(z,INT,r*c,2); s=AS(z); s[0]=r; s[1]=c;
 v=AV(z);
 ICPY(v,AV(y)+i*c,r*c);
 DQ(r, *v-=j; v+=c;);
 R z;
}    /* ((i+i.r){y)-"1 ({:$y){.j */

static DF2(jtcut2sx){PROLOG(0024);DECLF;A h=0,*hv,y,yy;B b,neg,pfx,*u,*v;C id;I d,e,hn,m,n,p,t,yn,*yu,*yv;P*ap;V*vf;
 PREF2(jtcut2sx);
 SETIC(w,n); t=AT(w); m=(I)sv->localuse.lvp[0]; neg=0>m; pfx=m==1||m==-1; b=neg&&pfx;  // m = n from u;.n
 RZ(a=a==mark?eps(w,take(num(pfx?1:-1),w)):DENSE&AT(a)?sparse1(a):a);
 ASSERT(n==*AS(a),EVLENGTH);
 ap=PAV(a);
 if(!(equ(num(0),SPA(ap,e))&&AN(SPA(ap,a))))R cut2(cvt(B01,a),w,self); 
 vf=VAV(fs);
 if(VGERL&sv->flag){h=sv->fgh[2]; hv=AAV(h); hn=AN(h); id=0;}else id=vf->id; 
 y=SPA(ap,i); yn=AN(y); yv=AV(y); u=v=BAV(SPA(ap,x)); e=m=0;
 GATV0(yy,INT,yn+1,1); yu=AV(yy); *yu++=p=pfx?n:-1;
 switch(pfx+(I )(id==CLEFT||id==CRIGHT||id==CCOMMA?2:0)){
  case 0:          DQ(yn, if(*v){++m;      *yu++=  yv[v-u];              } ++v;); break;
  case 1: v+=yn-1; DQ(yn, if(*v){++m;      *yu++=  yv[v-u];              } --v;); break;
  case 2:          DQ(yn, if(*v){++m; d=p; *yu++=p=yv[v-u]; e=MAX(e,p-d);} ++v;); break;
  case 3: v+=yn-1; DQ(yn, if(*v){++m; d=p; *yu++=p=yv[v-u]; e=MAX(e,d-p);} --v;);
 }
 yu=AV(yy); p=pfx?yu[m]:0;
 if(t&DENSE){C*wv;I c,k,r,*s;
  r=MAX(1,AR(w)); s=AS(w); wv=CAV(w); c=aii(w); k=c<<bplg(t); 
  CUTSWITCH(EACHCUTSP)
 }else if(id==CPOUND){A z;I i,*zi; 
  GATV0(z,INT,m,1); zi=AV(z); 
  if(pfx)for(i=m;i>=1;--i)*zi++=(yu[i-1]-yu[i  ])-neg;
  else   for(i=1;i<=m;++i)*zi++=(yu[i  ]-yu[i-1])-neg;
  EPILOG(z);
 }else{A a,ww,x,y,z,*za,zz,z0;I c,i,j,q,qn,r;P*wp,*wwp;
  wp=PAV(w); a=SPA(wp,a); x=SPA(wp,x); y=SPA(wp,i); yv=AV(y); r=*AS(y); c=*(1+AS(y));
  RZ(ww=cps(w)); wwp=PAV(ww);
  GATV0(z,BOX,m,1); za=AAV(z);
  switch(AN(a)&&*AV(a)?2+pfx:pfx){
   case 0:
    p=yu[0]; DO(r, if(p<=yv[c*i]){p=i; break;});
    for(i=1;i<=m;++i){
     j=yu[i]; DO(q=r-p, if(j<yv[c*(p+i)]){q=i; break;}); qn=q;
     if(neg)DO(qn=r-p, if(j-1<yv[c*(p+i)]){qn=i; break;});
     *AS(ww)=(yu[i]-yu[i-1])-neg; 
     SPB(wwp,i,sely(y,qn,p,1+yu[i-1]));
     SPB(wwp,x,selx(x,qn,p));
     RZ(*za++=zz=h?df1(z0,ww,hv[(i-1)%hn]):CALL1(f1,ww,fs));
     // reallocate ww if it was used, which we detect by seeing the usecount incremented.  This requires that everything that
     // touches a buffer either copy it or rat().  So that ] doesn't have to rat(), we also detect reuse here if the same buffer
     // is returned to us
     p+=q; if(WASINCORP1(zz,ww)){RZ(ww=cps(w)); wwp=PAV(ww);}
    }
    break;
   case 1:
    p=yu[m]; DO(r, if(p<=yv[c*i]){p=i; break;});
    for(i=m;i>=1;--i){
     j=yu[i-1]; DO(q=r-p, if(j<=yv[c*(p+i)]){q=i; break;}); qn=q;
     if(neg){j=yu[i]; qn=0; DO(r-p, if(j<yv[c*(p+i)]){qn=q-i; break;});}
     *AS(ww)=(yu[i-1]-yu[i])-neg; 
     SPB(wwp,i,sely(y,qn,p+q-qn,yu[i]+neg));
     SPB(wwp,x,selx(x,qn,p+q-qn));
     RZ(*za++=zz=h?df1(z0,ww,hv[(m-i)%hn]):CALL1(f1,ww,fs));
     p+=q; if(WASINCORP1(zz,ww)){RZ(ww=cps(w)); wwp=PAV(ww);}
    }
    break;
   case 2:
    for(i=1;i<=m;++i){
     q=yu[i]-yu[i-1]; *AS(ww)=q-neg;
     SPB(wwp,x,irs2(apv(q-neg,p,1L),x,0L,1L,-1L,jtfrom));
     RZ(*za++=zz=h?df1(z0,ww,hv[(i-1)%hn]):CALL1(f1,ww,fs));
     p+=q; if(WASINCORP1(zz,ww)){RZ(ww=cps(w)); wwp=PAV(ww);}
    }
    break;
   case 3:
    for(i=m;i>=1;--i){
     q=yu[i-1]-yu[i]; *AS(ww)=q-neg;
     SPB(wwp,x,irs2(apv(q-neg,p+neg,1L),x,0L,1L,-1L,jtfrom));
     RZ(*za++=zz=h?df1(z0,ww,hv[(i-1)%hn]):CALL1(f1,ww,fs));
     p+=q; if(WASINCORP1(zz,ww)){RZ(ww=cps(w)); wwp=PAV(ww);}
    }
    break;
  }
  z=ope(z);
  EPILOG(z);
}}   /* sparse f;.n (dense or sparse) */

// sv is u;.n *zz is ??  result is ??
static C*jtidenv0(J jt,A a,A w,V*sv,I zt,A*zz){A fs,y,z;
 *zz=0; 
 fs=sv->fgh[0];
 RE(df1(y,num(0),iden(VAV(fs)->fgh[0])));
 if(TYPESLT(zt,AT(y))){*zz=df1(z,cut2(a,w,cut(ds(CBOX),sv->fgh[1])),amp(fs,ds(COPE))); R 0;}  // fgh still has the original A, OK to use
 if(TYPESGT(zt,AT(y)))RE(y=cvt(zt,y)); 
 R CAV(y);
}    /* pointer to identity element */

/* locals in cut2:                       */
/* b    1 iff _1 cut                     */
/* c    atoms in an item of w            */
/* d    adjusted length of current cut   */
/* e    max width of a cut               */
/* h    gerund                           */
/* hv   gerund                           */
/* id   function code                    */
/* k    # bytes in an item of w          */
/* m    # of cuts                        */
/* n    #a and #w                        */
/* neg  1 iff _1 or _2 cut               */
/* p    remaining length in a            */
/* pfx  1 iff 1 or _1 cut                */
/* q    length of current cut            */
/* sep  the cut character                */
/* u    ptr to a for next    cut         */
/* v    ptr to a for current cut         */
/* v1   ptr to w for current cut         */
/* wd   non0 iff w is relative              */

// the values are 1 byte, either 8 bits or 1 bit
#define FRETLOOPBYTE(decl,compI,comp) \
{ \
 UC val=*(UC*)fret, *avv=(UC*)av;  /* compare value, pointer to input */ \
 decl /* other variables needed */ \
 d=-pfx; I nleft=n;  /* if prefix, first omitted fret starts at length 0; set number of items yet to process */ \
 /* The search step finds one fret and consumes it.  For pfx, the first count is the #items BEFORE the first fret, \
 subsequent items are the interfret distance, and the last is the length of the remnant PLUS 1 to account for the \
 final fret which is in the last partition.  For !pfx, the first count INCLUDES the first fret, subsequent items are interfret \
 distance, and the remnant at the end AFTER that last fret is discarded. */ \
 while(nleft){I cmpres=0;  /* =0 for the warning */ \
  /* for byte-at-a-time searches we can save some time by skipping 4/8 bytes at a time */ \
  while(nleft>=SZI){I avvI = *(I*)avv; /* read one word */ \
   compI  /* Code (if any) to convert avvI to a value that is non0 iff there is a match */ \
   if(avvI!=0){  /* if we can't skip,  exit loop and search byte by byte */ \
    I skiphalf=((avvI&IHALF0)==0)<<(LGSZI-1); avv+=skiphalf; d+=skiphalf; nleft-=skiphalf;  /* if first half empty, skip over it - remove if unaligned load penalty */ \
    break; \
   } \
   avv+=SZI; d+=SZI; nleft-=SZI;  /* skip the whole 8 bytes */ \
  } \
  if(!nleft)break;  /* if we skipped over everything, we're through */ \
  I testct=BW; testct=(nleft>testct)?testct:nleft;  /* testct=# compares to do, BW max */ \
  /* rattle off compares; save the number in cmpres, MSB=1st compare.  We keep the count the same for prediction */ \
  I testi=testct; do{UI match=(comp); ++avv; cmpres=2*cmpres+match;}while(--testi); \
  /* process them out of cmpres, writing lengths */ \
  testi=testct;  /* save # cells processed */ \
  cmpres<<=BW-testct;  /* if we didn't shift in BW bits, move the first one we did shift to the MSB */ \
  while(cmpres){ \
   UI4 ctz; CTLZI(cmpres,ctz); I len=BW-ctz; testct-=len; d+=len; /* get # leading bits including the 1; decr count of unprocessed bits; set d=length of next field to output */ \
   if(d<255)*pd++ = (UC)d; else{*pd++ = 255; *(UI4*)pd=(UI4)d; pd+=SZUI4; m-=SZUI4;}  /* write out encoded length; keep track of # long fields emitted */ \
   if(pd>=pdend){RZ(pd0=jtgetnewpd(jt,pd,pd0)); pdend=(C*)CUTFRETEND(pd0); pd=CUTFRETFRETS(pd0);}  /* if we filled the current buffer, get a new one */ \
   cmpres<<=1; cmpres<<=(len-=1); d=0;   /* discard bit up to & incl the fret; clear the carryover of #cells in partition */ \
  } \
  d += testct;  /* add in any bits not shifted out of cmpres as going into d */ \
  nleft -= testi;  /* decr number of cells to do */ \
 } \
}

// Template for comparisons without byte-wide checks
#define FRETLOOPNONBYTE(decl, comp) \
{ \
 decl \
 d=-pfx; I nleft=n;  /* if prefix, first omitted fret starts at length 0; set number of items yet to process */ \
 /* The search step finds one fret and consumes it.  For pfx, the first count is the #items BEFORE the first fret, \
 subsequent items are the interfret distance, and the last is the length of the remnant PLUS 1 to account for the \
 final fret which is in the last partition.  For !pfx, the first count INCLUDES the first fret, subsequent items are interfret \
 distance, and the remnant at the end AFTER that last fret is discarded. */ \
 while(nleft){I cmpres=0;  /* =0 for the warning */ \
  I testct=BW; testct=(nleft>testct)?testct:nleft;  /* testct=# compares to do, BW max */ \
  /* rattle off compares; save the number in cmpres, MSB=1st compare.  We keep the count the same for prediction */ \
  I testi=testct; do{comp   cmpres=2*cmpres+match;}while(--testi); \
  /* process them out of cmpres, writing lengths */ \
  testi=testct;  /* save # cells processed */ \
  cmpres<<=BW-testct;  /* if we didn't shift in BW bits, move the first one we did shift to the MSB */ \
  while(cmpres){ \
   UI4 ctz; CTLZI(cmpres,ctz); I len=BW-ctz; testct-=len; d+=len; /* get # leading bits including the 1; decr count of unprocessed bits; set d=length of next field to output */ \
   if(d<255)*pd++ = (UC)d; else{*pd++ = 255; *(UI4*)pd=(UI4)d; pd+=SZUI4; m-=SZUI4;}  /* write out encoded length; keep track of # long fields emitted */ \
   if(pd>=pdend){RZ(pd0=jtgetnewpd(jt,pd,pd0)); pdend=(C*)CUTFRETEND(pd0); pd=CUTFRETFRETS(pd0);}  /* if we filled the current buffer, get a new one */ \
   cmpres<<=1; cmpres<<=(len-=1); d=0;   /* discard bits up to & incl the fret; clear the carryover of #cells in partition */ \
  } \
  d += testct;  /* add in any bits not shifted out of cmpres as going into d */ \
  nleft -= testi;  /* decr number of cells to do */ \
 } \
}

// the values are intolerant values that match a machine variable size
#define FRETLOOPSGL(T) FRETLOOPNONBYTE(T val=*(T*)fret; T *avv=(T*)av;  , \
  UI match=(val==*avv); ++avv; \
)

// the values are D, which use tolerance
#define FRETLOOPSGLD FRETLOOPNONBYTE(D val=*(D*)fret; D *avv=(D*)av; D cct=jt->cct;  , \
  UI match=(TCMPEQB(cct,val,*avv)); ++avv; \
)

// 1st word in buf is chain, 2nd is end+1 of data
#define EACHCUT(stmt) \
 do{UC *pdend=(UC*)CUTFRETEND(pd0);   /* 1st ele is # eles; get &chain  */ \
  while(pd<pdend){   /* step to first/next; process each fret.  Quit when pointing to end */ \
   UI len=*pd++; if(len==255){len=*(UI4*)pd; pd+=SZUI4;} d=len-neg;  /* fetch size, adjust if neg */ \
   stmt;  /* perform user function */  \
   v1 += len*k;  /* advance to next fret position in w */ \
  }  /* loop till buffer all processed */ \
  if(!(pd0=CUTFRETCHAIN(pd0)))break;  /* step to next buffer; exit if none */ \
  pd=CUTFRETFRETS(pd0);  /* point to 1st d */  \
 }while(1);  /* when buffer exhausted, chain to next; stop when no next */


// allocate a new fret buffer and chain it to the old one
// pd is &last element+1 (i. e. endptr of current buffer)
// pd0 is &current buffer
// result is &new buffer, or 0 if error
// we fill in the chain and length fields in the current buffer, and set the length field of the new
// buffer to the size we allocated
#define FRETALLOSIZE (512-10)  // number of frets to allocate at a time - if they are long type (i. e. I).  For frets less than 255
   // items long, we can store 4 or 8 times as many
static A jtgetnewpd(J jt, UC* pd, A pd0){A new;
 CUTFRETEND(pd0)=(I)pd;  // fill in size of current buffer
 GATV0(new,INT,FRETALLOSIZE,0);  // get a new buffer.  rank immaterial.  Use GATV because of large size
 CUTFRETCHAIN(pd0)=new;  // chain the new block to the old
 pd0=new;  // step pd0 to new block
 CUTFRETEND(pd0)=(I)(CUTFRETFRETS(pd0)+(FRETALLOSIZE*SZI)-10);  // return value: endptr of new buffer
 R pd0;  // return address of data in the new block
}


DF2(jtcut2){F2PREFIP;PROLOG(0025);A fs,z,zz;I neg,pfx;C id,*v1,*wv,*zc;I cger[128/SZI];
     I ak,at,wcn,d,k,m=0,n,r,wt,*zi;I d1[32]; A pd0; UC *pd, *pdend;  // Don't make d1 too big - it fill lots of stack space
 PREF2(jtcut2);
 if((SGNIF(AT(a),SB01X)|-(AT(w)&SPARSE))<0)R cut2sx(a,w,self);
#define ZZFLAGWORD state
 I state=0;  // init flags, including zz flags

 SETIC(w,n); wt=AT(w);   // n=#items of w; wt=type of w
 r=MAX(1,AR(w)); wv=CAV(w); wcn=aii(w); k=wcn<<bplg(wt);   // r=rank>.1, s->w shape, wv->w data, wcn=#atoms in cell of w, k=#bytes in cell of w;
 // If the verb is a gerund, it comes in through h, otherwise the verb comes through f.  Set up for the two cases
 if(!(VGERL&FAV(self)->flag)){
  fs=FAV(self)->fgh[0];  // the verb we will execute
  id=FAV(fs)->id;  // fetch its pseudocharacter
 }else{
  RZ(fs=createcycliciterator((A)&cger, self));  // use a verb that cycles through the gerunds.
  id=0;  // set an invalid pseudochar id for the gerund, to indicate 'not a primitive'
 }
 if(FAV(fs)->mr>=r){
  // we are going to execute f without any lower rank loop.  Thus we can use the BOXATOP etc flags here.  These flags are used only if we go through the full assemble path
  state = (FAV(fs)->flag2&VF2BOXATOP1)>>(VF2BOXATOP1X-ZZFLAGBOXATOPX);  // Don't touch fs yet, since we might not loop
  state &= ~((FAV(fs)->flag2&VF2ATOPOPEN1)>>(VF2ATOPOPEN1X-ZZFLAGBOXATOPX));  // We don't handle &.> here; ignore it
  state |= (-state) & (I)jtinplace & (ZZFLAGWILLBEOPENED|ZZFLAGCOUNTITEMS); // remember if this verb is followed by > or ; - only if we BOXATOP, to avoid invalid flag setting at assembly
 }
 AF f1=FAV(fs)->valencefns[0];  // point to the action routine now that we have handled gerunds

 // Time to find the frets.  If a is the impossible type INT+LIT, a contains the frets already, in the single buffer (that means we are acting on behalf of Key /.)
 if(FAV(self)->id==CCUT){   // see if we are acting on behalf of /.  Fall through if not
  pfx=(I)FAV(self)->localuse.lvp[0]; neg=SGNTO0(pfx); pfx&=1;  // neg=cut type is _1/_2; pfx=cut type is 1/_1
  if(a!=mark){  // dyadic forms
   if(((AN(a)-1)&(-n))<0){  // empty x, do one call on the entire w if y is non-empty
    R CALL1(f1,w,fs);
   }
   if(((-AN(a))&(SGNIF(AT(a),BOXX)))<0)R cut2bx(a,w,self);  // handle boxed a separately if a not empty
   if(!(B01&AT(a)))RZ(a=cvt(B01,a));  // convert other a to binary, error if impossible
   if(!AR(a))RZ(a=reshape(sc(n),a));   // extend scalar x to length of y
   ak=1; at=B01;  // cell of a is 1 byte, and it's Boolean
  }else{
   // monadic forms.  If we can handle the type/length here, leave it; otherwise convert to Boolean.
   // If w is Boolean, we have to pretend it's LIT so we use the correct fret value rather than hardwired 1
   if((((wt&(B01|LIT|INT|FL|C2T|C4T|SBT))-1)|(k-1)|(((BW==32&&wt&FL&&k==SZD)-1)&((k&-k&(2*SZI-1))-k)))>=0){a=w; ak=k; at=(wt+B01)&~B01;  // monadic forms: if w is an immediate type we can handle, and the length is a machine-word length, use w unchanged
   }else{RZ(a=n?eps(w,take(num(pfx?1:-1),w)):mtv); ak=1; at=B01;}  // any other w, replace by w e. {.w (or {: w).  Set ak to the length of a cell of a, in bytes.  Empty cells of w go through here to convert to list
  }
  {I x; ASSERT(n==SETIC(a,x),EVLENGTH);}

  // *fret is value to match; n is #items to match; pd0=&d1, pd->d1 fretarea pdend=&d1[max+1] (pointers into current fret buffer) k=item length av->data to compare
  pd0=(A)&d1; pd=CUTFRETFRETS(pd0); pdend=(C*)&d1+sizeof(d1)-10;  // pd0, pd, pdend start out set for first buffer

  void *av=voidAV(a);  // point to the start of the fret data
  void *fret=(C*)av+ak*((n-1)&(pfx-1));  // point to the fret
  // The type of a is always one we can handle here - other types have been converted to B01.  B01 types look for 1, others look for fret value.  Select routine based on length/tolerance/byte-boolean
  I rtnx = CTTZ(k); rtnx=(at&B01)?4:rtnx; rtnx=(at&FL)?5:rtnx;  // 0-3=bytes, 4=B01, 5=FL
  switch(rtnx){
  case 0: // single bytes.  This is like the B01 case below but we cleverly detect noncomparing words by word-wide methods, and then convert the equality test into B01 format a word at a time
   {
    // In this loop d is the length of the fret
    I valI=((UC *)fret)[0]; valI|=valI<<8; valI|=valI<<16; if(BW==64)valI|=valI<<(BW/2);  // the fret value, replicated in each byte of the word
    // n bits 0..LGSZI-1 are from original n & are the number of valid bits overflowing into a partial word.  Bits LGSZI..LGSZI+LGBB-1 are the (shifted) # words to process
    n+=(n&(SZI-1))?SZI:0; UI *wvv=(UI*)av; UI bits=*wvv++;  // prime the pipeline for top of loop.  Bias n to have the number of words we need to visit, even partially
    d=1-pfx; // If first fret is in position 0, that's length 0 for prefix, length 1 for suffix
    while(n>0){    // where we load bits SZI at a time
     // skip empty words, to get best speed on near-zero a.  This exits with the first unskipped word in bits.  The test XORs the target with bits, then looks for presence of any 0 byte.
     // 0 byte is detected by subtracting 1 from each byte's LSB and seeing if that causes the byte's MSB to switch from 0 to 1.  If any does, stop & process.  If not, skip the word.  Always leave 1 word so we have a batch to process
     while(1){if(n<(2*SZI))break; I bytecomp=valI^bits; if(((~bytecomp)&(bytecomp-(I)0x0101010101010101))&(I)0x8080808080808080)break;    bits=*wvv++; d+=SZI; n-=SZI;}
     UI bitstack;  // the bits packed together and processed one by one
     {I batchsize=n>>LGSZI; batchsize=MIN(BB,batchsize);  // batch size, never 0
     // XOR to put 0 bytes on frets; combine bits so that a boolean bit is 0 only if all bits of its byte are 0, i. e. 0 only if fret
     bitstack=0; while(--batchsize>0){I bits2=*wvv++; bits ^=valI; ZBYTESTOZBITS(bits); bits&=VALIDBOOLEAN; PACKBITSINTO(bits,bitstack); bits=bits2;};  // process all but the last in batch; keep read pipe ahead. There is a carried dependency over PACKBITS, but the next word's bits can overlap it
     }// Handle the last word of the batch.  It might have non-Boolean data at the end, AFTER the Boolean padding.  Just clear the non-boolean part in this line
     bits ^=valI; ZBYTESTOZBITS(bits); bits&=VALIDBOOLEAN; PACKBITSINTO(bits,bitstack);
     bitstack=~bitstack;     // Convert not-a-fret bits to fret bits
     // Now handle the last batch, by discarding garbage bits at the end and then shifting the lead bit down to bit 0
     if(n>=BW+SZI)bits=*wvv++;else {n-=n&(SZI-1)?SZI:0; bitstack<<=(BW-n)&(SZI-1); bitstack>>=BW-n;}  // Unbias n back to actual count (always in [1,64]); discard invalid trailing bits; shift leading byte to position 0.  For non-last batches, start on next batch. (Don't fetch outside block!)
     while(bitstack){
      {I bitfound=CTTZI(bitstack); d+=bitfound;   // d=length of new partition; prevend=location of new partition
      if(d<255)*pd++ = (UC)d; else{*pd++ = 255; *(UI4*)pd=(UI4)d; pd+=SZUI4; m-=SZUI4;}  /* write out encoded length; keep track of # long fields emitted */
      d=-bitfound;  // save the position of the found bit.  Do this before the subroutine, so bitfound doesn't need to be saved
      if(pd>=pdend){RZ(pd0=jtgetnewpd(jt,pd,pd0)); pdend=(C*)CUTFRETEND(pd0); pd=CUTFRETFRETS(pd0);}  /* if we filled the current buffer, get a new one */
      }
      bitstack&=bitstack-1;
     }
     d+=BW;  // trailing zeros add to d, but the count wraps around.  Net, add the batch to d
     n-=BW;  // account for 1 batch of bits
    };
    // end with d=length of last partition, 
    d+=n-1;  // n is nonpositive.  This removes from d the nonexistent bits of the batch.  the -1 is for compatibility with other branches, which end with the length of the last partition 1 short, corrected for below.  This changes d back to length of fret
   }
   break;
  case 1: // 2 bytes
   FRETLOOPSGL(US) break;
  case 2: // 4 bytes
   FRETLOOPSGL(UI4) break;
#if BW==64
  case 3: // 8 bytes
   FRETLOOPSGL(UI) break;
#endif
  case 4: // single-byte Boolean, looking for 1s
   {
    // In this loop d is the length of the fret
    // n bits 0..LGSZI-1 are from original n & are the number of valid bits overflowing into a partial word.  Bits LGSZI..LGSZI+LGBB-1 are the (shifted) # words to process
    n+=(n&(SZI-1))?SZI:0; UI *wvv=(UI*)av; UI bits=*wvv++;  // prime the pipeline for top of loop.  Bias n to have the number of words we need to visit, even partially
    d=1-pfx; // If first fret is in position 0, that's length 0 for prefix, length 1 for suffix
    while(n>0){    // where we load bits SZI at a time
     // skip empty words, to get best speed on near-zero a.  This exits with the first unskipped word in bits
     while(bits==0 && n>=(2*SZI)){bits=*wvv++; d+=SZI; n-=SZI;}  // fast-forward over zeros.  Always leave 1 word so we have a batch to process
     UI bitstack;  // the bits packed together and processed one by one
     {I batchsize=n>>LGSZI; batchsize=MIN(BB,batchsize);  // batch size, never 0
     bitstack=0; while(--batchsize>0){I bits2=*wvv++; PACKBITSINTO(bits,bitstack); bits=bits2;};  // process all but the last in batch; keep read pipe ahead. There is a carried dependency over PACKBITS, but the next word's bits can overlap it
     }// Handle the last word of the batch.  It might have non-Boolean data at the end, AFTER the Boolean padding.  Just clear the non-boolean part in this line
     bits&=VALIDBOOLEAN; PACKBITSINTO(bits,bitstack);
     // Now handle the last batch, by discarding garbage bits at the end and then shifting the lead bit down to bit 0
     if(n>=BW+SZI)bits=*wvv++;else {n-=n&(SZI-1)?SZI:0; bitstack<<=(BW-n)&(SZI-1); bitstack>>=BW-n;}  // Unbias n back to actual count (always in [1,64]); discard invalid trailing bits; shift leading byte to position 0.  For non-last batches, start on next batch. (Don't fetch outside block!)
     while(bitstack){
      {I bitfound=CTTZI(bitstack); d+=bitfound;   // d=length of new partition; prevend=location of new partition
      if(d<255)*pd++ = (UC)d; else{*pd++ = 255; *(UI4*)pd=(UI4)d; pd+=SZUI4; m-=SZUI4;}  /* write out encoded length; keep track of # long fields emitted */
      d=-bitfound;  // save the position of the found bit.  Do this before the subroutine, so bitfound doesn't need to be saved
      if(pd>=pdend){RZ(pd0=jtgetnewpd(jt,pd,pd0)); pdend=(C*)CUTFRETEND(pd0); pd=CUTFRETFRETS(pd0);}  /* if we filled the current buffer, get a new one */
      }
      bitstack&=bitstack-1;
     }
     d+=BW;  // trailing zeros add to d, but the count wraps around.  Net, add the batch to d
     n-=BW;  // account for 1 batch of bits
    };
    // end with d=length of last partition, 
    d+=n-1;  // n is nonpositive.  This removes from d the nonexistent bits of the batch.  the -1 is for compatibility with other branches, which end with the length of the last partition 1 short, corrected for below.  This changes d back to length of fret
   }
   break;
  case 5: // float (tolerant)
   FRETLOOPSGLD break;
  }

  // partition sizes have been calculated.
  // prepare for the looping.  We need:
  // pd0 pointing to first buffer
  // pd->first d to process.  We skip the first d if pfx
  // m has # d values
  // v1->first w-cell to process, taking into account skipped cells owing to neg and omitted headers

  // if pfx, emit the runout field for the last d (if suffix these are discarded).  We must add one to d
  // to include the prefix in the length of d
  if(pfx){++d; if(d<255)*pd++ = (UC)d; else{*pd++ = 255; *(UI4*)pd=(UI4)d; pd+=SZUI4; m-=SZUI4;}}
  // close the last d buffer
  CUTFRETCHAIN(pd0)=0; CUTFRETEND(pd0)=(I)pd;  // set end-of-chain and endptr in last d buffer
  // chase the chains to count the number of d values.  Each is one byte long, except for the long ones which are 1+SZI bytes.  But we subtracted SZI from m for
  // every long encoding, so if we just total the number of bytes in the blocks and add it to m, we will have the number of d-values
  for(pd0=(A)&d1; pd0; pd0=CUTFRETCHAIN(pd0))m += (UC*)CUTFRETEND(pd0)-CUTFRETFRETS(pd0);  // add in bufend-bufstart
  // set pointers to first d and first cell
  v1=wv; pd0=(A)&d1; pd=CUTFRETFRETS(pd0);
  // if pfx, skip over the first d and any w-cells indicated in it.  Also skip over 1 more w-cell if neg indicates the first cell should be skipped
  if(pfx){
   UI len=*pd++; if(len==255){len=*(UI4*)pd; pd+=SZUI4;}  // get first d and skip over it
   v1+=(len+neg)*k;  // skip over the before-first-prefix cells, and the fret cell if neg
   --m;  // remove the discarded d from the count
  }
 }else{
  // Here we are processing /.; the original a had the fret sizes built already.  Just set up the loop variables based on them
  m=CUTFRETCOUNT(a);  // # frets, set by caller
  pd0=a;  // &first block of frets
  pd=CUTFRETFRETS(a);  // &first fret
  CUTFRETCHAIN(a)=0;  // indicate only 1 block in use.  Note this overlaps with AK & thus cannot be set until pd extracted
  // NOTE!!! is the operation is retried, m must be restored into CUTFRETCOUNT
  v1=wv;  // we always start with the first cell
     // CUTFRETEND in input block is set by caller
  pfx=1; neg=0;  // This is a ;.1 cut
 }

 // At this point we have m, the number of result cells; pd0, pointer to first block of lengths; pd, pointer to first fret-length to use; v1, pointer to first participating cell of w; pfx, neg

 // process, handling special cases
 zz=0;   // indicate no result from special cases
 switch(id){
  case CPOUND:
   GATV0(zz,INT,m,1); zi=AV(zz); EACHCUT(*zi++=d;); 
   break;
  case CDOLLAR:
   GATV0(zz,INT,m,1); zi=AV(zz); EACHCUT(*zi++=d;); A zw=vec(INT,MAX(0,r-1),AS(w)+1);
   R IRS2(zz,zw,0L,0L,1L,jtover,z);
  case CHEAD:
   GA(zz,wt,m*wcn,r,AS(w)); zc=CAV(zz); AS(zz)[0]=m;
   EACHCUT(if(d)MC(zc,v1,k); else fillv(wt,wcn,zc); zc+=k;);
   break;
  case CTAIL:
   GA(zz,wt,m*wcn,r,AS(w)); zc=CAV(zz); AS(zz)[0]=m;
   EACHCUT(if(d)MC(zc,v1+k*(d-1),k); else fillv(wt,wcn,zc); zc+=k;);
   break;
// scaf MUST CALCULATE e or discard this, which might be better
// scaf should take this under BOXATOP?
  case CSLASH:
   {
   VARPS adocv; varps(adocv,fs,wt,0);  // qualify the operation, returning action routine and conversion info
   if(adocv.f){C*z0=0,*zc;I t,zk,zt;  // if the operation is a primitive that we can  apply / to...  z0 will hold a neutral if we have to calculate one
    zt=rtype(adocv.cv);
#if SY_64
    GA(zz,zt,m*wcn,r,AS(w)); AS(zz)[0]=m; 
#else
    // plusinsI writes past the end of its result area if d==1 and there is overflow (normally that would be OK and would be converted to D inplace).  Here it overruns the buffer,
    // so we allocate one extra word just in case
    GA(zz,zt,m*wcn+1,r,AS(w)); AS(zz)[0]=m; AN(zz)=m*wcn;
#endif
    if(!AN(zz))R zz;  // don't run function on empty arg
    I atomsize=bpnoun(zt);
    zc=CAV(zz); zk=wcn*atomsize;
    if((t=atype(adocv.cv))&&TYPESNE(t,wt)){RZ(w=cvt(t,w)); wv=CAV(w);}
    I rc=EVOK;   // accumulate error code
    EACHCUT(if(d){ I lrc=((AHDRRFN*)adocv.f)(wcn,d,(I)1,v1,zc,jt); rc=lrc<rc?lrc:rc;} else{if(!z0){z0=idenv0(a,w,FAV(self),zt,&z); // compared to normal reduces, c means d and d means n
        if(!z0){if(z)R z; else{rc=jt->jerr; break;}}} mvc(zk,zc,atomsize,z0);} zc+=zk;
    );
    if(255&rc){
     jsignal(rc);
     if(FAV(self)->id!=CCUT)CUTFRETCOUNT(a)=m;  // if we are going to retry, we have to reset the # frets indicator which has been destroyed
     R rc>=EWOV?cut2(a,w,self):0;
    }else R adocv.cv&VRI+VRD?cvz(adocv.cv,zz):zz;
    break;
    }
  }
 }

 // If we didn't get a result from the special cases, run the normal result loop
 if(!zz){
  if(m){
   // There are cells.  Run the result loop over them
   // Allocate the virtual block we will use for arguments
   A virtw; fauxblock(virtwfaux); fauxvirtual(virtw,virtwfaux,w,r,ACUC1|ACINPLACE);  // allocate UNINCORPORABLE block
   // Copy in the shape of a cell.  The number of cells in a subarray will depend on d
   MCISH(AS(virtw)+1,AS(w)+1,r-1);
   // Set the offset to the first data
   AK(virtw)=v1-(C*)virtw;  // v1 is set to point to starting cell; transfer that info
   // Remove WILLOPEN for the callee.  We use the caller's WILLOPEN status for the result created here
   // Remove inplacing if the verb is not inplaceable, possible because we always set u;. to inplaceable so we can get the WILLBEOPENED flags
   jtinplace = (J)(intptr_t)((I)jtinplace & (~(JTWILLBEOPENED+JTCOUNTITEMS+JTINPLACEA)) & ((((wt&TYPEVIPOK)!=0)&REPSGN(AC(w))&(FAV(fs)->flag>>(VJTFLGOK1X-JTINPLACEWX)))*JTINPLACEW-(JTINPLACEW<<1)));  // turn off inplacing unless DIRECT and w is inplaceable

#define ZZDECL
#include "result.h"
   ZZPARMS(1,m,1)
#define ZZINSTALLFRAME(optr) *optr++=m;
   I gerundx=0;  // if we have gerunds, this indicates which one we should run next

   do{UC *pdend=(UC*)CUTFRETEND(pd0);   /* 1st ele is # eles; get &chain  */
    while(pd<pdend){   /* step to first/next; process each fret.  Quit when pointing to end */
     UI len=*pd++; if(len==255){len=*(UI4*)pd; pd+=SZUI4;} d=len-neg;  /* fetch size, adjust if neg */
     AS(virtw)[0]=d; AN(virtw)=wcn*d; // install the size of the partition into the virtual block, and # atoms
     // call the user's function
     AC(virtw)=ACUC1|ACINPLACE;   // in case we created a virtual block from it, restore inplaceability to the UNINCORPABLE block
     RZ(z=CALL1IP(f1,virtw,fs));  //normal case

#define ZZBODY  // assemble results
#include "result.h"

     AK(virtw) += len*k;   // advance to data position of next fret
    }  /* loop till buffer all processed */
    if(!(pd0=CUTFRETCHAIN(pd0)))break;  /* step to next buffer; exit if none */
    pd=CUTFRETFRETS(pd0);  /* point to 1st d */
   }while(1);  /* when buffer exhausted, chain to next; stop when no next */

#define ZZEXIT  // handle fill & dissimilar results
#include "result.h"

  }else{
   // No frets.  Apply the operand to 0 items; return (0,$result) $ result (or $,'' if error on fill-cell).  The call is non-inplaceable
   RZ(z=reitem(zeroionei(0),w));  // create 0 items of the type of w
   UC d=jt->uflags.us.cx.cx_c.db; jt->uflags.us.cx.cx_c.db=0; zz=CALL1(f1,z,fs); jt->uflags.us.cx.cx_c.db=d; if(EMSK(jt->jerr)&EXIGENTERROR)RZ(zz); RESETERR;
   RZ(zz=reshape(over(zeroionei(0),shape(zz?zz:mtv)),zz?zz:zeroionei(0)));
  }
 }
 EPILOG(zz);
}    /* f;.1  f;._1  f;.2  f;._2  monad and dyad */


static DF1(jtcut1){R cut2(mark,w,self);}

#if 0   // never executed
#define PSCASE(id,zt,wt)    ((id)+256*(zt)+1024*(wt))
#define PSLOOP(Tz,Tw,F,v0)      \
    {B*u;Tw* RESTRICT wv;Tz s=v0,x,* RESTRICT zv;                   \
     GA(z,zt,n,1,0);                            \
     u=m+av; wv=m+(Tw*)AV(w); zv=m+(Tz*)AV(z);  \
     switch(pfx+2*(id==CBSLASH)){               \
      case 0: DQ(n, x=*--wv; if(*--u)s=v0; *--zv=F;              ); break;  /* <@(f/\.);.2 */  \
      case 1: DQ(n, x=*--wv;               *--zv=F; if(*--u)s=v0;); break;  /* <@(f/\.);.1 */  \
      case 2: DQ(n, x=*wv++;               *zv++=F; if(*u++)s=v0;); break;  /* <@(f/\ );.2 */  \
      case 3: DQ(n, x=*wv++; if(*u++)s=v0; *zv++=F;              ); break;  /* <@(f/\ );.1 */  \
    }}

static A jtpartfscan(J jt,A a,A w,I cv,B pfx,C id,C ie){A z=0;B*av;I m,n,zt;
 n=AN(w); m=id==CBSDOT?n:0; zt=rtype(cv); av=BAV(a);
 switch(PSCASE(ie,CTTZ(zt),CTTZ(AT(w)))){
  case PSCASE(CPLUS,   INTX,B01X):       PSLOOP(I,B,s+=x,      0   );       break;
  case PSCASE(CPLUS,   FLX, FLX ): NAN0; PSLOOP(D,D,s+=x,      0.0 ); NAN1; break;
  case PSCASE(CMAX,    INTX,INTX):       PSLOOP(I,I,s=MAX(s,x),IMIN);       break;
  case PSCASE(CMAX,    FLX, FLX ):       PSLOOP(D,D,s=MAX(s,x),-inf);       break;
  case PSCASE(CMIN,    INTX,INTX):       PSLOOP(I,I,s=MIN(s,x),IMAX);       break;
  case PSCASE(CMIN,    FLX, FLX ):       PSLOOP(D,D,s=MIN(s,x),inf );       break;
  case PSCASE(CMAX,    B01X,B01X):
  case PSCASE(CPLUSDOT,B01X,B01X):       PSLOOP(B,B,s|=x,      0   );       break;
  case PSCASE(CMIN,    B01X,B01X):
  case PSCASE(CSTARDOT,B01X,B01X):       PSLOOP(B,B,s&=x,      1   );       break;
  case PSCASE(CNE,     B01X,B01X):       PSLOOP(B,B,s^=x,      0   );       break;
  case PSCASE(CEQ,     B01X,B01X):       PSLOOP(B,B,s=s==x,    1   );       break;
 }
 R z;
}    /* [: ; <@(ie/\);.k  on vector w */
#endif

// ;@((<@(f/\));._2 _1 1 2) when  f is atomic   also @: but only when no rank loop required  also \. for \
// also [: ; (<@(f/\));._2 _1 1 2)  when no rank loop required
// NOTE: if there are no cuts, this routine produces different results from the normal routine if the operation is one we recognise.
//  This routine produces an extra axis, as if the shape of the boxed result were preserved even when there are no boxed results
DF2(jtrazecut2){A fs,gs,y,z=0;B b; I neg,pfx;C id,sep,*u,*v,*wv,*zv;I d,k,m=0,wi,p,q,r,*s,wt;
    V *vv;VARPS adocv;
 RZ(a&&w);
 gs=FAV(self)->fgh[1+(CFORK==FAV(self)->id)]; vv=VAV(gs); y=vv->fgh[0]; fs=VAV(y)->fgh[1];  // self is ;@:(<@(f/\);.1)     gs  gs is <@(f/\);.1   y is <@(f/\)  fs is   f/\  ...
 p=SETIC(w,wi); wt=AT(w); k=(I)vv->localuse.lvp[0]; neg=0>k; pfx=k==1||k==-1; b=neg&&pfx;   // p,wi is # items of w; 
 id=FAV(fs)->id;  // fs is f/id   where id is \ \.
  // if f is atomic/\ or atomic /\., set ado and cv with info for the operation
// obsolete  if(id==CBSLASH)adocv = vapfx(FAV(FAV(fs)->fgh[0])->fgh[0],wt);   // FAV(fs)->fgh[0] is f/    FAV(FAV(fs)->fgh[0])->fgh[0] is f
// obsolete  else           adocv = vasfx(FAV(FAV(fs)->fgh[0])->fgh[0],wt); 
 varps(adocv,fs,wt,1+(id!=CBSLASH));   // fs is f/\  type 1 is f/\ 2 is f/\.
 if(SPARSE&AT(w)||!adocv.f)R jtspecialatoprestart(jt,a,w,self);  // if sparse w or nonatomic function, do it the long way
 if(a!=mark){   // dyadic case
  if((-AN(a)&((AR(a)^1)-1)&-(AT(a)&B01+SB01))>=0)R jtspecialatoprestart(jt,a,w,self);  // if a is not nonempty boolean list, do it the long way.  This handles ;@: when a has rank>1
  // a is nonempty boolean list
  if(AT(a)&SB01)RZ(a=cvt(B01,a));
  v=CAV(a); sep=C1;
 }else if(((AR(w)-2)&-(wt&IS1BYTE))<0){a=w; v=CAV(a); sep=v[(wi-1)&(pfx-1)];}  // monad.  Create char list of frets: here if 1-byte list/atom
 else{RZ(a=wi?eps(w,take(num((pfx<<1)-1),w)):mtv); v=CAV(a); sep=C1;}   // here if other types/shapes
 // v-> byte list of frets, sep is the fret char
 ASSERT(wi==SETIC(a,r),EVLENGTH);
 r=MAX(1,AR(w)); s=AS(w); wv=CAV(w); d=aii(w); k=d<<bplg(wt);  // d=#atoms in an item of w
 if(pfx){u=v+wi; while(u>v&&sep!=*v)++v; p=u-v;}
 I t,zk,zt;                     /* atomic function f/\ or f/\. */
 if((t=atype(adocv.cv))&&TYPESNE(t,wt)){RZ(w=cvt(t,w)); wv=CAV(w);}
 zt=rtype(adocv.cv); zk=d<<bplg(zt);
// ?? vaid(VAV(fs)->fgh[0]) is always /, so this does nothing
// unused if(1==r&&!neg&&B01&AT(a)&&p==wi&&v[(wi-1)&(pfx-1)]){RE(z=partfscan(a,w,adocv.cv,(B)pfx,id,vaid(VAV(fs)->fgh[0]))); if(z)SEGFAULT /* scaf  R z;*/}
 GA(z,zt,AN(w),r,s); zv=CAV(z); // allocate size of w, which is as big as it can get if there are no discarded items
 while(p){I n;
  if(u=memchr(v+pfx,sep,p-pfx))u+=pfx^1; else{if(!pfx)break; u=v+p;}
  q=u-v;
  if(n=q-neg){  // number of items in this section
   I rc=EVOK;
   if(d)rc=((AHDRPFN*)adocv.f)(d,n,(I)1,wv+k*(b+wi-p),zv,jt);  // do the prefix, but not if items empty
   if(rc&255){jsignal(rc); R rc>=EWOV?razecut2(a,w,self):0;}  // if overflow, restart the whole thing with conversion to float
   m+=n; zv+=n*zk; 
  }
  p-=q; v=u;  
 }
 *AS(z)=m; AN(z)=m*d; R adocv.cv&VRI+VRD?cvz(adocv.cv,z):z;
}   

DF1(jtrazecut1){R razecut2(mark,w,self);}

// if pv given, it is the place to put the shapes of the top 2 result axes.  If omitted, do them all and return an A block for them
// if pv is given, set pv[0] to 0 if there is a 0 anywhere in the result frame
// we look at all the axes even if we don't store them all; if any are 0 we set
// n is the op type (as in u;.n)
static A jttesos(J jt,A a,A w,I n, I *pv){A p;I*av,c,axisct,k,m,s,*ws;
 RZ(a&&w);
 axisct=c=AS(a)[1]; av=AV(a); ws=AS(w);
 if(pv){c=(c>2)?2:c; p=0; // if more than 2 axes requested, limit the return to that
 }else{GATV0(p,INT,c,1); pv=AV(p); AS(p)[0]=c;}  // all requested, make an A block for it
 if(n>0)DO(axisct, m=av[i]; s=ws[i]; if(!((I)p|(m = m?(s+m-1)/m:1&&s)))pv[0]=0; if(i<c)pv[i]=m;)  // ;.3, calculate # sections
 else   DO(axisct, m=av[i]; k=av[axisct+i]; s=ws[i]-ABS(k); if(!((I)p|(m = 0>s?0:m?(I )(k||s%m)+s/m:1)))pv[0]=0; if(i<c)pv[i]=m;);
 R p;
}    /* tesselation result outer shape */


static F2(jttesa){A x;I*av,ac,c,d,k,r,*s,t,*u,*v;
 RZ(a&&w);
 t=AT(a);
 RZ(a=vib(a));    // convert a to integer (possibly with infinities)
 r=AR(a); s=AS(a); ac=c=r?s[r-1]:1; av=AV(a); d=AR(w);  // r = rank of x; s->shape of x; c=#axes specd in x, av->data; d=rank of w
 ASSERT(d>=c&&(2>r||2==s[0]),EVLENGTH);  // x must not be bigger than called for by rank of w, and must be a list or 2-item table
 if(2<=r)DO(c, ASSERT(0<=av[i],EVDOMAIN););  // if movement vector given, it must be nonnegative
 if(2==r&&t&INT){RETF(a);}  // if we can use a as given, return a as is
 GATV0(x,INT,2*c,2); s=AS(x); s[0]=2; s[1]=c;  // allocate space for start/stride, only for axes that will be modified.  We will modify it
 u=AV(x); v=u+c; s=AS(w);  // u->movement vector, v->length
 if(2==r)DO(c,   *u++=av[i]; k=av[i+ac]; if(k&((I)IMIN>>1)){k=(s[i]^REPSGN(k))-REPSGN(k);} *v++=k;);  // if k infinite, make length the axis length, + or -
 if(2> r)DO(c,   *u++=1;     k=av[i];  if(k&((I)IMIN>>1)){k=(s[i]^REPSGN(k))-REPSGN(k);} *v++=k;);
 RETF(x);
}    /* tesselation standardized left argument */

// These bits overlap with those used for ;.0
#define STATEREFLECTX 0x400  // subarray must be reflected
#define STATEREFLECTY 0x800
#define STATETAKE 0x1000  // subarray must be shortened at the end
static DF2(jttess2){A z,zz=0,virtw,strip;I n,rs[3],cellatoms,cellbytes,vmv,hmv,vsz,hsz,hss,hds,vss1,vss,vds1,vds,vlrc,vtrc,lrchsiz,hi,vi,vkeep1,vtrunc,hkeep1,htrunc;C *svh,*dvh;
 PROLOG(600); PREF2(jttess2);   // enforce left rank 2
#define ZZFLAGWORD state
 I state;
 RZ(a=tesa(a,w));   // expand x to canonical form, with trailing axes-in-full deleted
 n=(I)FAV(self)->localuse.lvp[0]; state=(~n)&STATETAKE;  // n=op type (as in u;.n); set TAKE bit if code=3: we will shorten out-of-bounds args
 I wr=AR(w); I wt=AT(w); // rank of w, type of w
 I *as=AS(a), *av=IAV(a), axisct=as[1];  // a-> shape of a, axisct=# axes in a, av->mv/size area
 // get shape of final result
 tesos(a,w,n,rs);      // vert/horiz shape of 1st 2 axes of result, from argument sizes and cut type
 I nregcells; PROD(nregcells,axisct,av+axisct);  // get size of a region, not including taken-in-full axes.  Must be no bigger than # atoms in w
 if((-(wt&DENSE)&(-wr)&(-AN(w))&(-nregcells)&(-AN(a))&(-rs[0]))>=0){A p;  // if any of those is 0...
  // w is sparse, atomic, or empty, or the region has no axes or is empty, or the result is empty.  Go the slow way: create a selector block for each cell, and then apply u;.0 to each cell
  // trailing axes taken in full will be omitted from the shape of the result
  RZ(p=tesos(a,w,n,0));  // recalculate all the result shapes
  A za, zw; RZ(za=cant1(tymesW(head(a),cant1(abase2(p,iota(p)))))); RZ(zw=tail(a));
  RETF(cut02(IRS2(za, zw,0L,1L,1L,jtlamin2,z),w,self));  // ((|: ({.a) * |: (#: i.)p) ,:"1 ({:a)) u;.0 w
 }
 DECLF;  // get the function pointers
 fauxblockINT(xfaux,5,1); // declare xpose arg where it has scope
 I *ws=AS(w);  // ws-> shape of w
 // get address of end+1 of the source data, so we can avoid out-of-bounds
 C *sdataend=CAV(w)+(AN(w)<<bplg(wt));  // addr+1 of source data
 I inrecursion=(n+256)&512;   // 512 if bit 8 and 9 of n have different values, otherwise 0
 if(axisct>2) {A next2;  // we build the u for the next 2 axes here
  // We do two axes of tessellation at a time.  If there are more than two axes, replace the verb with a verb that will reapply
  // the tessellation lacking the first two axes.  Unfortunately this differs considerably between ;.3 and ;._3
  // The ;._3 cut is pretty efficient, moving the data only once per pair of axes added.  The ;.3 cut creates argument cells, which
  // is inefficient; but that case will be very rare
  next2=qq(amp(drop(v2(0,2),a),cut(ds((((4-axisct)|n)<0)?CRIGHT:CBOX),sc(n^256^(inrecursion>>1)))),num(-2));  // RIGHT if n<0 or axisct>4
  if(n<0){
   // ;._3, the more usual and faster case
   // we will recur on ((0 2}.x)&(];.n)"_2 to build up _2-cells of the final result.  To save a smidgen, we will suppress the final
   // transpose during the recursion by using a large value for n; we will undo all the transposes at once during the last return
   // build the recursion down 2 levels: (0 2}.x)&(];.n)"_2
   // If this is the top level of recursion, prefix it with (original cut)"wr@:(??...&|:)@:
   if(!inrecursion){
    // The shape returned from ];.n is a mixture of subarray shapes Sn and result shapes Rn.  The first two result-shape values R0 and R1
    // are implied, as they are encoded in the position of the current cell.  The other shapes are interleaved:
    // 3 axes: S0 S1 R2 S2 [rest of ws beyond the axes]
    // 4 axes: S0 S1 R3 R2 S2 S3 [rest of ws beyond the axes]
    // 5 axes: S0 S1 R3 R2 S2 S3 R4 S4 [rest of ws beyond the axes]
    // these must be transposed to result in R2..Rn S0..Sn Wx..Wn
    A xposearg; fauxINT(xposearg,xfaux,wr+axisct-2,1) I *xpv=IAV(xposearg);
    DO((axisct>>1)-1, *xpv++ = 4*i+3; *xpv++=4*i+2;) if(axisct&1)*xpv++=2*axisct-4;  // Rn
    DO(axisct>>1, *xpv++ = 4*i+0; *xpv++=4*i+1;) if(axisct&1)*xpv++=2*axisct-3;  // Sn
    DO(wr-axisct, *xpv++=2*axisct+i-2;);  // Wn, all the rest
    next2=atco(atco(qq(fs,sc(wr)),amp(xposearg,ds(CCANT))),next2);  // combine it all
   }
  }else{
   // ;.3.  The cells coming out of the lower tessellations may have dissimilar shape so we have to box them
   // If the next level is the last, it becomes (0 2}.x)&(<;.n)"_2  - adding one boxing level.  Otherwise use ] instead of < above
   next2=qq(amp(drop(v2(0,2),a),cut(ds(CBOX),sc(n^256^(inrecursion>>1)))),num(-2));
   // collect the components that contribute to a single input-to-u, one box for each : (<@:>"2)@:(0 1&|:)@:next2
   next2=atco(atco(qq(atco(ds(CBOX),ds(COPE)),num(2)),amp(v2(0,1),ds(CCANT))),next2);
   if(!inrecursion){
    // at the top level, add on u@>
    next2=atco(atop(fs,ds(COPE)),next2);
   }
  }
  fs=next2; f1=FAV(fs)->valencefns[0];   // get the function corresponding to the new verb
 }
 // Now that we really know what fs is, see if it is a BOXATOP form
 state |= ((FAV(fs)->mr>=wr?VF2BOXATOP1:0)&FAV(fs)->flag2)>>(VF2BOXATOP1X-ZZFLAGBOXATOPX);  // If this is BOXATOP and it will generate a single box, set so for result loop.  Don't touch fs yet, since we might not loop
 state &= ~((FAV(fs)->flag2&VF2ATOPOPEN1)>>(VF2ATOPOPEN1X-ZZFLAGBOXATOPX));  // We don't handle &.> here; ignore it
 // We don't honor WILLBEOPENED; perhaps we should

 // Start preparing for loop
 I axisproc=axisct; axisproc=axisct>2?2:axisproc;  // number of axes to process here: 1 or 2
 vmv=av[0]; vsz=av[axisct+0];  // v movement vector and size
 PROD(cellatoms,wr-axisproc,ws+axisproc); cellbytes = cellatoms<<bplg(wt);  // get # atoms in cell, then #bytes
 // Figure out the size of the cells we will be making blocks of.  They have all but the first 1 or 2 axes of a/w
 fauxblock(virtwfaux);
 if(axisproc==1){
  // Here there is just one axis.  We will never have to move anything.
  // Set up the variables for the 2-D loop, most of which we don't need
  vds=cellbytes*vmv;  // the amount by which we advance the virtual-block pointer: 1 row 
  rs[1]=1;  // one horizontal iteration
  hsz=1;  // treat horiz size as one cell
  htrunc=0; hkeep1=hsz; hmv=0;  // no truncation owing to horizontal position
  vtrc=vlrc=0;  // stifle all copying to the strip, which doesn't exist
  hss=hds=lrchsiz=0; vds1=vss1=0; vss=0; svh=dvh=0; // no horiz looping, so this is immaterial
  vss=0;   // since we never loop back in horiz loop, this is immaterial

  fauxvirtual(virtw,virtwfaux,w,wr,ACUC1);  // allocate the virtual block, pointing into w
 }else{
  // Here there are at least 2 axes.  We will process the first 2, carefully laying the cells into memory to require minimal copying of data
  // We will advance along the leading axis, building up a tall narrow strip of cells.  When we advance along the second axis, we will be able to
  // reuse the overlapping parts of the strip.

  hmv=av[1];  hsz=av[axisct+1];    // h movement vector and size
  state|=REPSGN(vsz)&STATEREFLECTY; state|=REPSGN(hsz)&STATEREFLECTX; 
  vsz=(vsz<0)?-vsz:vsz; hsz=(hsz<0)?-hsz:hsz;  // take abs(size)
  // Calculate the amount to add to the strip position when we move along the second axis.  If there is no overlap in the second axis, just move 0.
  hss = hmv*cellbytes; I hdsc=(hmv>=hsz)?0:hmv;  // horiz source stride, dest stride in cells.  A mv of 0 must have the same effect as an mv of infinity
  // Calculate the amount to add to the strip position when we move along the leading axis.  If there is no overlap on either axis, move 0.  If horiz overlap only, move vsize.  If vert overlap, move vert mv
  vss1 = cellbytes*ws[1]; vss=vss1*vmv; vds1=cellbytes*hsz; I vdsc=(hdsc==0)?0:vsz; vdsc=(((vmv-1)|(vsz-vmv-1))<0)?vdsc:vmv;   // if vmv<=0 || vsz-vmv<=0
                           // vert source stride to move 1 row, stride betw cells, dest stride 1 row (in bytes), 1 result cell (in dest rows)
  // number of rows to move to dest when going down 1 cell
  vlrc=vmv; vlrc=(vlrc>vsz)?vsz:vlrc;  // vert pos of lower-right corner.
  lrchsiz=hmv; lrchsiz=(hdsc==0)?hsz:lrchsiz;   // length uncovered by a move, but size if that is 0 (meaning no move & thus no left side)
  vds = vdsc*vds1;  hds=hdsc*cellbytes; lrchsiz*=cellbytes; // make into byte counts
  // figure out how many (if any) of the cells for each axis are going to be truncated; and what the size of the first truncated block will be
  if(state&STATETAKE){I space, nfit;  // 3-cut, which may have shards - calcuate horizontal shard info
   // space after first; divide by mv; count how many fit including the first; get vi when we hit truncation; see how many items are valid in 1st truncated cell 
   space=ws[1]-hsz; nfit=hmv?(space/hmv):0; nfit=(space<0)?0:nfit+1; htrunc=rs[1]-nfit; hkeep1=ws[1]-nfit*hmv;
  }
  // see how many atoms are needed in the strip.  There needs to be enough for one top, plus all the bottoms, plus destination stride for each horizontal move
  I stripn = (hsz*(vsz-vlrc+vlrc*rs[0])+hdsc*rs[1])*cellatoms;  // total number of atoms in the strip
  // Allocate the area we will use for the strip
  GA(strip,wt,stripn,0,0);  // allocate strip - rank immaterial
  // Allocate the virtual block we will use to address subarrays
  fauxvirtual(virtw,virtwfaux,strip,wr,ACUC1);
  // NOTE: if we use a virtual block, it will ra() the (initially empty) strip which will make it recursive.  Then, when indirect elements are moved in (without ra()), they are subject
  // to premature free at the end.  To prevent this we set the strip to have no atoms, so that nothing moved into it will be freed.  Since strip is not itself used as an argument to
  // anything, this is OK.
  AN(strip)=0;

  // Move in the left side of the first strip (the left edge of first column)
  svh=CAV(w); dvh=CAV(strip);   // ?vh=pointer to top-left of first column
  C *svb=svh; C *dvb=dvh;  // running pointers used to fill the strip
  // Start with the first cell, above the bottom part.  Copy the left-hand side (length vds1-lrchsiz).
  vtrc=vsz-vlrc;  // number of lines to copy in top half
  DQ(vtrc, I sh=sdataend-(svb+vds1-lrchsiz); sh=(sh>=0)?0:sh; sh+=vds1-lrchsiz; sh=(sh<0)?0:sh; MC(dvb,svb,sh); svb+=vss1; dvb+=vds1;);  // copies to dest are all sequential; sequential rows of src
  // copy the bottom-left part of the first cell, and of all cells if the cells overlap (i. e. vds!=0) (length vds1-lrchsiz)
  DQ(vds?rs[0]:1, C *svb1=svb; DQ(vlrc, I sh=sdataend-(svb1+vds1-lrchsiz); sh=(sh>=0)?0:sh; sh+=vds1-lrchsiz; sh=(sh<0)?0:sh; MC(dvb,svb1,sh); svb1+=vss1; dvb+=vds1;); svb+=vss;);  // sequential to dest; hop to cells of src
  // advance the horiz pointers, which now point to the top-left of the top cell, to the top-left of the top-right area of that cell
  svh+=(vds1-lrchsiz); dvh+=(vds1-lrchsiz);
 }
 // The rest is the same for 1- and 2-axis cut
 I hvirtofst=AK(virtw); I * RESTRICT virtws=AS(virtw);  // save initial offset (to top-left of source)
 // Install shape/item count into the virtual array.  First install height; then width if given; then the rest of the shape of w
 *virtws++=vsz; if(axisproc>1){*virtws++=hsz;} MCISH(virtws,ws+axisproc,wr-axisproc) AN(virtw) = vsz*hsz*cellatoms;
 // calculate vertical shard info & see if there are any shards
 if(state&STATETAKE){I space, nfit;  // 3-cut, which may have shards
  // space after first; divide by mv; count how many fit including the first; get vi when we hit truncation; see how many items are valid in 1st truncated cell 
  space=ws[0]-vsz; nfit=vmv?(space/vmv):0; nfit=(space<0)?0:nfit+1; vtrunc=rs[0]-nfit; vkeep1=ws[0]-nfit*vmv;
  if(vtrunc<=0&&htrunc<=0)state &= ~STATETAKE;  // if we don't have any shards after all, turn off looking for them
 }
 // for each horizontal position, move in the right edge and process

#define ZZDECL
#include "result.h"
 I mn=rs[0]*rs[1];  // number of cells in the result
  ZZPARMS(axisproc,mn,1)
#define ZZINSTALLFRAME(optr) *optr++=rs[axisproc-1];if(axisproc>1)*optr++=rs[0];  // Note the 2-axis result is transposed, so shape is reversed here for that case

 for(hi=rs[1];hi;--hi){C *svv, *dvv;  // pointers within the lower-right corner, starting at the top-left of the corner.  Before that we use them to advance through top-right
  // move in the top-right part of top cell, stopping before the lower-right corner.
  svv=svh; dvv=dvh; // init to top-left of right edge (length=lcrhsiz)
  DQ(vtrc, I sh=sdataend-(svv+lrchsiz); sh=(sh>=0)?0:sh; sh+=lrchsiz; sh=(sh<0)?0:sh; MC(dvv,svv,sh); svv+=vss1; dvv+=vds1;);
  // now ?vv points to top-left of lower-right quadrant of top cell
  // point the virtual-block pointer to the top of the row
  AK(virtw)=hvirtofst;
  // move horiz calc outside loop
  I hkeep=hkeep1-(htrunc-hi)*hmv;  // horiz length to keep
  // for each cell in the vertical strip, move in the lower-right corner and execute
  for(vi=rs[0];vi;--vi){A opcell=virtw;
   // move in the lower-right corner.  Avoid overrun, and detect if there was overrun.  Watch out for addresses wrapping around the end of memory (length=lcrhsiz)
   C *dvlrc=dvv, *svlrc=svv; DQ(vlrc, I sh=sdataend-(svlrc+lrchsiz); sh=(sh>=0)?0:sh; sh+=lrchsiz; sh=(sh<0)?0:sh; MC(dvlrc,svlrc,sh); svlrc+=vss1; dvlrc+=vds1;);
   // If a copy was truncated, take only the valid elements.
   if(state&(STATEREFLECTX|STATEREFLECTY|STATETAKE)){  // something might be truncated/reflected
    I vkeep=vkeep1-(vtrunc-vi)*vmv; vkeep=(vkeep>vsz)?vsz:vkeep; // vertical length to keep
    if(((vkeep-vsz)|(hkeep-hsz))<0){   // if either axis must be shortened...
     RZ(opcell=take((hkeep-hsz)>=0?sc(vkeep):v2(vkeep,hkeep),virtw));
    }
    if(state&STATEREFLECTY)RZ(opcell=reverse(opcell));  // reverse vertical
    if(state&STATEREFLECTX)RZ(opcell=df1(z,opcell,qq(ds(CREV),num(-1))));  // reverse horizontal
   }
   // execute the verb on the cell and assemble results
   RZ(z=CALL1(f1,opcell,fs)); 

#define ZZBODY  // assemble results
#include "result.h"

   // advance to next vertical (i. e. first-axis) position
   AK(virtw)+=vds; svv+=vss; dvv+=vds;
  }
  // advance to next horizontal position
  svh+=hss; dvh+=hds; hvirtofst+=hds;
 }

#define ZZEXIT
#include "result.h"

// if we took 2 axes, they are transposed in the result.  reverse the transpose. - unless this is a recursive call and we are deferring the transpose until the top level because n is negative
 if(!(((axisproc<<9)|(inrecursion&n))&512)){A xposeaxes;  // axisproc=1, or bit 9 of n mismatches bit 8 (with n>0), is enough to turn off transpose
  RZ(xposeaxes=apvwr(AR(zz),0L,1L)); IAV(xposeaxes)[0]=1;  IAV(xposeaxes)[1]=0;  // xpose arg, 1 0 2 3 4...
  RZ(zz=cant2(xposeaxes,zz));
 }

 EPILOG(zz);

}

static DF1(jttess1){A s;I m,r,*v;
 RZ(w);
 r=AR(w); RZ(s=shape(w)); rifvs(s); v=AV(s);
 m=IMAX; DO(r, if(m>v[i])m=v[i];); DO(r, v[i]=m;);  // Get length of long axis; set all axes to that length in a arg to cut
 R tess2(s,w,self);
}


F2(jtcut){A h=0,z;I flag=0,k;
// NOTE: u/. is processed using the code for u;.1 and passing the self for /. into the cut verb.  So, the self produced
// by /. and ;.1 must be the same as far as flags etc.  For the shared case, inplacing is OK
 RZ(a&&w);
 ASSERT(NOUN&AT(w),EVDOMAIN);
 RE(k=i0(w));
 if(NOUN&AT(a)){flag=VGERL; RZ(h=fxeachv(1L,a)); ASSERT(-2<=k&&k<=2,EVNONCE);}
 switch(k){
 case 0:          z=fdef(0,CCUT,VERB, jtcut01,jtcut02, a,w,h, flag|VJTFLGOK2, RMAX,2L,RMAX); break;
 case 1: case -1:
 case 2: case -2: if(!(NOUN&AT(a)))flag=VJTFLGOK2+VJTFLGOK1; z=fdef(0,CCUT,VERB, jtcut1, jtcut2,  a,w,h, flag, RMAX,1L,RMAX); break;
 case 3: case -3: case 259: case -259: z=fdef(0,CCUT,VERB, jttess1,jttess2, a,w,h, flag, RMAX,2L,RMAX); break;
 default:         ASSERT(0,EVDOMAIN);
 }
 RZ(z);
 FAV(z)->localuse.lvp[0]=(void *)k;  // remember the integer form of the cut selector
 R z;
}
