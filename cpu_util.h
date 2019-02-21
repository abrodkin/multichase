/* Copyright 2015 Google Inc. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PORT_H_INCLUDED
#define PORT_H_INCLUDED

// We assume 1024 bytes is good enough alignment to avoid false sharing on all
// architectures.
#define AVOID_FALSE_SHARING     (1024)

#if defined(__x86_64__) || defined(__i386__)
static inline void cpu_relax(void) {
  asm volatile("rep; nop");
}
#elif defined __powerpc__
static inline void cpu_relax(void) {
  // HMT_low()
  asm volatile("or 1,1,1");
  // HMT_medium()
  asm volatile("or 2,2,2");
  // barrier()
  asm volatile("" : : : "memory");
}
#elif defined(__aarch64__)
# define cpu_relax() asm volatile("yield" ::: "memory")
#elif defined(__arc__)
#define cpu_relax() asm volatile("" ::: "memory");
#else
#warning "no cpu_relax for your cpu"
#define cpu_relax() do {} while (0)
#endif

#ifndef CACHELINE_SIZE
#define CACHELINE_SIZE 64
#endif

#endif
