cmake_minimum_required(VERSION 3.17)

project(J C ASM)

set(jsrc ${CMAKE_SOURCE_DIR}/../jsrc)

set(jconsole_srcs
  jconsole.c
  jeload.c
)

set(libj_srcs
  a.c
  ab.c
  aes-c.c
  aes-sse2.c
  af.c
  ai.c
  am.c
  am1.c
  amn.c
  ao.c
  ap.c
  ar.c
  as.c
  au.c
  blis/gemm_c-ref.c
  blis/gemm_int-aarch64.c
  blis/gemm_int-avx.c
  blis/gemm_int-sse2.c
  blis/gemm_vec-ref.c
  c.c
  ca.c
  cc.c
  cd.c
  cf.c
  cg.c
  ch.c
  cip.c
  cl.c
  cp.c
  cpdtsp.c
  cpuinfo.c
  cr.c
  crs.c
  ct.c
  cu.c
  cv.c
  cx.c
  d.c
  dc.c
  dss.c
  dstop.c
  dsusp.c
  dtoa.c
  f.c
  f2.c
  fbu.c
  gemm.c
  i.c
  io.c
  j.c
  jdlllic.c
  k.c
  m.c
  mbx.c
  p.c
  pv.c
  px.c
  r.c
  rl.c
  rt.c
  s.c
  sc.c
  sl.c
  sn.c
  t.c
  u.c
  v.c
  v0.c
  v1.c
  v2.c
  va1.c
  va1ss.c
  va2.c
  va2s.c
  va2ss.c
  vamultsp.c
  vb.c
  vbang.c
  vbit.c
  vcant.c
  vchar.c
  vcat.c
  vcatsp.c
  vcomp.c
  vcompsc.c
  vd.c
  vdx.c
  ve.c
  vf.c
  vfft.c
  vfrom.c
  vfromsp.c
  vg.c
  vgauss.c
  vgcomp.c
  vgranking.c
  vgsort.c
  vgsp.c
  vi.c
  viavx.c
  viix.c
  visp.c
  vm.c
  vo.c
  vp.c
  vq.c
  vrand.c
  vrep.c
  vs.c
  vsb.c
  vt.c
  vu.c
  vx.c
  vz.c
  w.c
  wc.c
  wn.c
  ws.c
  x.c
  x15.c
  xa.c
  xaes.c
  xb.c
  xc.c
  xcrc.c
  xd.c
  xf.c
  xfmt.c
  xh.c
  xi.c
  xl.c
  xo.c
  xs.c
  xsha.c
  xt.c
  xu.c
  openssl/sha/keccak1600.c
  openssl/sha/md4_dgst.c
  openssl/sha/md4_one.c
  openssl/sha/md5_dgst.c
  openssl/sha/md5_one.c
  openssl/sha/openssl-util.c
  openssl/sha/sha1_one.c
  openssl/sha/sha256.c
  openssl/sha/sha3.c
  openssl/sha/sha512.c
  # asm_linux_srcs
  openssl/sha/asm/keccak1600-x86_64-elf.S
  openssl/sha/asm/sha1-x86_64-elf.S
  openssl/sha/asm/sha256-x86_64-elf.S
  openssl/sha/asm/sha512-x86_64-elf.S
  # aes_ni_srcs
  aes-ni.c
)


if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
  add_compile_options(
    -O2
    -fPIC
    -fvisibility=hidden
    -fwrapv
    -fno-strict-aliasing
    -ffp-contract=fast
    -Wextra
    -Werror
    -Wno-unused-parameter
    -Wno-parentheses
    -Wno-pointer-sign
    -Wno-unused-value
    -Wno-string-plus-int
    -Wno-empty-body
  )
elseif (CMAKE_C_COMPILER_ID STREQUAL "Intel")
 add_compile_options(
    -O3
    -fPIC
    -fvisibility=hidden
    -fwrapv
    -fno-strict-aliasing
    #-no-vec  -no-simd
    -fp-model precise
    #-fimf-use-svml=false
 )
else () # if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  add_compile_options(
    -O2
    -fPIC
    -fvisibility=hidden
    -fwrapv
    -fno-strict-aliasing
    -Wextra
    -Werror
    -Wno-maybe-uninitialized
    -Wno-return-local-addr
    -Wno-implicit-fallthrough
    -Wno-cast-function-type
    -Wno-sign-compare
    -Wno-shift-negative-value
    -Wno-clobbered
  )
endif ()

if (NOT j64x)
  if (DEFINED ENV{j64x})
    set(j64x $ENV{j64x})
  else()
    message(STATUS "j64x unset - defaulting to j64")
    set(j64x j64)
  endif ()
endif ()
message(STATUS "Targeting ${j64x}")

list(TRANSFORM jconsole_srcs PREPEND ${jsrc}/)
list(TRANSFORM libj_srcs PREPEND ${jsrc}/)

add_executable(jconsole ${jconsole_srcs})
target_link_libraries(jconsole PRIVATE dl)
target_compile_definitions(jconsole PRIVATE READLINE)

add_library(j SHARED ${libj_srcs})
target_link_libraries(j PRIVATE m)
target_compile_options(j PRIVATE -maes)

if (j64x STREQUAL "j64avx")
  target_compile_definitions(j PRIVATE C_AVX=1)
  target_compile_options(j PRIVATE -mavx -mfma)
  target_sources(j PRIVATE ${jsrc}/blis/gemm_int-fma.c)
elseif (j64x STREQUAL "j64avx2")
  target_compile_definitions(j PRIVATE C_AVX=1 C_AVX2=1)
  target_compile_options(j PRIVATE -mavx2 -mfma)
  target_sources(j PRIVATE ${jsrc}/blis/gemm_int-fma.c)
endif()

if (USE_SLEEF AND j64x MATCHES "avx")
  message(STATUS "Using SLEEF")
  target_compile_definitions(j PRIVATE SLEEF=1)
  target_link_libraries(j PRIVATE ${jsrc}/../sleef/lib/linux/libsleef.a)
endif ()

if (USE_OpenMP)
  find_package(OpenMP REQUIRED)
  target_link_libraries(j PRIVATE OpenMP::OpenMP_C)
endif()

add_library(tsdll SHARED ${jsrc}/tsdll.c)
target_link_libraries(tsdll PRIVATE m)

install(TARGETS jconsole j tsdll DESTINATION ${jsrc}/../jlibrary/bin)
