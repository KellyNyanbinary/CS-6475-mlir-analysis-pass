target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx27.0.0"

define i32 @main() {
  call void @__assert_rtn()
  unreachable
}

declare void @__assert_rtn()
