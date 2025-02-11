#ifndef _SYS_SIGNAL_H_
#define _SYS_SIGNAL_H_

#include <sys/sigtypes.h>
#include <sys/siginfo.h>
#include <sys/time.h>

#define SIGHUP 1    /* hangup */
#define SIGINT 2    /* interrupt */
#define SIGQUIT 3   /* quit */
#define SIGILL 4    /* illegal instruction (not reset when caught) */
#define SIGTRAP 5   /* trace trap (not reset when caught) */
#define SIGABRT 6   /* abort() */
#define SIGFPE 8    /* floating point exception */
#define SIGKILL 9   /* kill (cannot be caught or ignored) */
#define SIGBUS 10   /* bus error */
#define SIGSEGV 11  /* segmentation violation */
#define SIGSYS 12   /* bad argument to system call */
#define SIGPIPE 13  /* write on a pipe with no one to read it */
#define SIGALRM 14  /* alarm clock */
#define SIGTERM 15  /* software termination signal from kill */
#define SIGSTOP 17  /* sendable stop signal not from tty */
#define SIGTSTP 18  /* stop signal from tty */
#define SIGCONT 19  /* continue a stopped process */
#define SIGCHLD 20  /* to parent on child stop or exit */
#define SIGTTIN 21  /* to readers pgrp upon background tty read */
#define SIGTTOU 22  /* like TTIN for output if (tp->t_local&LTOSTOP) */
#define SIGWINCH 28 /* window size changes */
#define SIGINFO 29  /* information request */
#define SIGUSR1 30  /* user defined signal 1 */
#define SIGUSR2 31  /* user defined signal 2 */
#define NSIG 32

typedef int sig_atomic_t;

typedef void (*sig_t)(int); /* type of signal function */

#define SIG_ERR ((sig_t)-1)
#define SIG_DFL ((sig_t)0)
#define SIG_IGN ((sig_t)1)

typedef struct sigaction {
  union {
    sig_t sa_handler;
    void (*sa_sigaction)(int, siginfo_t *, void *);
  };
  sigset_t sa_mask;
  int sa_flags;
} sigaction_t;

#define SA_ONSTACK 0x0001   /* take signal on signal stack */
#define SA_RESTART 0x0002   /* restart system call on signal return */
#define SA_RESETHAND 0x0004 /* reset to SIG_DFL when taking signal */
#define SA_NODEFER 0x0010   /* don't mask the signal we're delivering */
/* Only valid for SIGCHLD. */
#define SA_NOCLDSTOP 0x0008 /* do not generate SIGCHLD on child stop */
#define SA_NOCLDWAIT 0x0020 /* do not generate zombies on unwaited child */
#define SA_SIGINFO 0x0040   /* take sa_sigaction handler */

/* Flags for sigprocmask(): */
#define SIG_BLOCK 1   /* block specified signal set */
#define SIG_UNBLOCK 2 /* unblock specified signal set */
#define SIG_SETMASK 3 /* set specified signal set */

/*
 * Flags used with stack_t/struct sigaltstack.
 */
#define SS_ONSTACK 0x0001 /* take signals on alternate stack */
#define SS_DISABLE 0x0002 /* disable taking signals on alternate stack */

/* Flags for si_code field */
/* SIGILL */
#define ILL_ILLOPC 1 /* Illegal opcode */
#define ILL_ILLOPN 2 /* Illegal operand */
#define ILL_ILLADR 3 /* Illegal addressing mode */
#define ILL_ILLTRP 4 /* Illegal trap */
#define ILL_PRVOPC 5 /* Privileged opcode */
#define ILL_PRVREG 6 /* Privileged register */
#define ILL_COPROC 7 /* Coprocessor error */
#define ILL_BADSTK 8 /* Internal stack error */

/* SIGFPE */
#define FPE_INTDIV 1 /* Integer divide by zero */
#define FPE_INTOVF 2 /* Integer overflow */
#define FPE_FLTDIV 3 /* Floating point divide by zero */
#define FPE_FLTOVF 4 /* Floating point overflow */
#define FPE_FLTUND 5 /* Floating point underflow */
#define FPE_FLTRES 6 /* Floating point inexact result */
#define FPE_FLTINV 7 /* Invalid Floating point operation */
#define FPE_FLTSUB 8 /* Subscript out of range */

/* SIGSEGV */
#define SEGV_MAPERR 1 /* Address not mapped to object */
#define SEGV_ACCERR 2 /* Invalid permissions for mapped object*/

/* SIGBUS */
#define BUS_ADRALN 1 /* Invalid address alignment */
#define BUS_ADRERR 2 /* Non-existent physical address */
#define BUS_OBJERR 3 /* Object specific hardware error */

/* SIGTRAP */
#define TRAP_BRKPT 1 /* Process breakpoint */
#define TRAP_TRACE 2 /* Process trace trap */
#define TRAP_EXEC 3  /* Process exec trap */
#define TRAP_CHLD 4  /* Process child trap */
#define TRAP_LWP 5   /* Process lwp trap */
#define TRAP_DBREG 6 /* Process hardware debug register trap */
#define TRAP_SCE 7   /* Process syscall entry trap */
#define TRAP_SCX 8   /* Process syscall exit trap */

#ifdef _KERNEL

#include <stdbool.h>
#include <sys/cdefs.h>

/* Start and end address of signal trampoline that gets copied onto
 * the user's stack. */
extern char sigcode[];
extern char esigcode[];

typedef struct proc proc_t;
typedef struct pgrp pgrp_t;
typedef struct thread thread_t;
typedef struct ctx ctx_t;
typedef struct __ucontext ucontext_t;
typedef struct mcontext mcontext_t;
typedef struct syscall_result syscall_result_t;

/*! \brief Notify the parent of a change in the child's status.
 *
 * \note Must be called with p::p_lock and p->p_parent::p_lock held.
   Returns with both locks held.
 */
void sig_child(proc_t *p, int code);

/*! \brief Signal a process.
 *
 * Marks \a sig signal as pending, unless it's ignored by target process.
 * In most cases signal delivery wakes up the process. Signal handling is
 * usually performed in the context of target process.
 *
 * \sa sig_post
 * \note Must be called with p::p_lock held. Returns with p::p_lock held.
 */
void sig_kill(proc_t *p, ksiginfo_t *ksi);

/*! \brief Signal all processes in a process group.
 *
 * \note Must be called with pg::pg_lock held. Returns with pg::pg_lock held.
 */
void sig_pgkill(pgrp_t *pg, ksiginfo_t *ksi);

/*! \brief Determines which signal should be caught by current thread.
 *
 * Only signals with registered handlers can be caught.
 * If this function finds a signal that should stop or kill the current process,
 * it takes the appropriate action.
 *
 * \sa sig_post
 * \sa proc_stop
 * \sa sig_exit
 *
 * \returns signal number which should be caught or 0 if none */
int sig_check(thread_t *td, ksiginfo_t *ksi);

/*! \brief Do the setup necessary to catch a signal.
 *
 * The signal must have a registered handler.
 *
 * \note It's ok to call this procedure multiple times before returning
 * to userspace. The handlers will be called in reverse order of calls
 * to this procedure.
 * \note Must be called with current process's p_mtx acquired!
 */
void sig_post(ksiginfo_t *ksi);

/*! \brief Terminate the process as the result of posting a signal. */
__noreturn void sig_exit(thread_t *td, signo_t sig);

/*! \brief Delivers a hardware trap related signal to current thread. */
void sig_trap(signo_t sig, int code, void *addr, int trapno);

/*! \brief Prepare user context for entry to signal handler action.
 *
 * \note This is machine dependent code! */
int sig_send(signo_t sig, sigset_t *mask, sigaction_t *sa, ksiginfo_t *ksi);

/*! \brief Restore original user context after signal handler was invoked.
 *
 * \note This is machine dependent code! */
int do_sigreturn(ucontext_t *ucp);

/*! \brief Deliver signals to a process on return to userspace. */
void sig_userret(mcontext_t *ctx, syscall_result_t *result);

/*! \brief Reset handlers for caught signals on process exec.
 *
 * \note Must be called with p::p_lock held. */
void sig_onexec(proc_t *p);

/* System calls implementation. */
int do_sigaction(signo_t sig, const sigaction_t *act, sigaction_t *oldact);
int do_sigprocmask(int how, const sigset_t *set, sigset_t *oset);
int do_sigsuspend(proc_t *p, const sigset_t *mask);
int do_sigpending(proc_t *p, sigset_t *set);
int do_sigtimedwait(proc_t *p, sigset_t waitset, ksiginfo_t *kinfo,
                    struct timespec *timeout);

#endif /* !_KERNEL */

#endif /* !_SYS_SIGNAL_H_ */
