8450 // Shell.
8451 
8452 #include "types.h"
8453 #include "user.h"
8454 #include "fcntl.h"
8455 
8456 // Parsed command representation
8457 #define EXEC  1
8458 #define REDIR 2
8459 #define PIPE  3
8460 #define LIST  4
8461 #define BACK  5
8462 
8463 #define MAXARGS 10
8464 
8465 struct cmd {
8466   int type;
8467 };
8468 
8469 struct execcmd {
8470   int type;
8471   char *argv[MAXARGS];
8472   char *eargv[MAXARGS];
8473 };
8474 
8475 struct redircmd {
8476   int type;
8477   struct cmd *cmd;
8478   char *file;
8479   char *efile;
8480   int mode;
8481   int fd;
8482 };
8483 
8484 struct pipecmd {
8485   int type;
8486   struct cmd *left;
8487   struct cmd *right;
8488 };
8489 
8490 struct listcmd {
8491   int type;
8492   struct cmd *left;
8493   struct cmd *right;
8494 };
8495 
8496 struct backcmd {
8497   int type;
8498   struct cmd *cmd;
8499 };
8500 int fork1(void);  // Fork but panics on failure.
8501 void panic(char*);
8502 struct cmd *parsecmd(char*);
8503 
8504 // Execute cmd.  Never returns.
8505 void
8506 runcmd(struct cmd *cmd)
8507 {
8508   int p[2];
8509   struct backcmd *bcmd;
8510   struct execcmd *ecmd;
8511   struct listcmd *lcmd;
8512   struct pipecmd *pcmd;
8513   struct redircmd *rcmd;
8514 
8515   if(cmd == 0)
8516     exit();
8517 
8518   switch(cmd->type){
8519   default:
8520     panic("runcmd");
8521 
8522   case EXEC:
8523     ecmd = (struct execcmd*)cmd;
8524     if(ecmd->argv[0] == 0)
8525       exit();
8526     exec(ecmd->argv[0], ecmd->argv);
8527     printf(2, "exec %s failed\n", ecmd->argv[0]);
8528     break;
8529 
8530   case REDIR:
8531     rcmd = (struct redircmd*)cmd;
8532     close(rcmd->fd);
8533     if(open(rcmd->file, rcmd->mode) < 0){
8534       printf(2, "open %s failed\n", rcmd->file);
8535       exit();
8536     }
8537     runcmd(rcmd->cmd);
8538     break;
8539 
8540   case LIST:
8541     lcmd = (struct listcmd*)cmd;
8542     if(fork1() == 0)
8543       runcmd(lcmd->left);
8544     wait();
8545     runcmd(lcmd->right);
8546     break;
8547 
8548 
8549 
8550   case PIPE:
8551     pcmd = (struct pipecmd*)cmd;
8552     if(pipe(p) < 0)
8553       panic("pipe");
8554     if(fork1() == 0){
8555       close(1);
8556       dup(p[1]);
8557       close(p[0]);
8558       close(p[1]);
8559       runcmd(pcmd->left);
8560     }
8561     if(fork1() == 0){
8562       close(0);
8563       dup(p[0]);
8564       close(p[0]);
8565       close(p[1]);
8566       runcmd(pcmd->right);
8567     }
8568     close(p[0]);
8569     close(p[1]);
8570     wait();
8571     wait();
8572     break;
8573 
8574   case BACK:
8575     bcmd = (struct backcmd*)cmd;
8576     if(fork1() == 0)
8577       runcmd(bcmd->cmd);
8578     break;
8579   }
8580   exit();
8581 }
8582 
8583 int
8584 getcmd(char *buf, int nbuf)
8585 {
8586   printf(2, "$ ");
8587   memset(buf, 0, nbuf);
8588   gets(buf, nbuf);
8589   if(buf[0] == 0) // EOF
8590     return -1;
8591   return 0;
8592 }
8593 
8594 
8595 
8596 
8597 
8598 
8599 
8600 int
8601 main(void)
8602 {
8603   static char buf[100];
8604   int fd;
8605 
8606   // Assumes three file descriptors open.
8607   while((fd = open("console", O_RDWR)) >= 0){
8608     if(fd >= 3){
8609       close(fd);
8610       break;
8611     }
8612   }
8613 
8614   // Read and run input commands.
8615   while(getcmd(buf, sizeof(buf)) >= 0){
8616     if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
8617       // Clumsy but will have to do for now.
8618       // Chdir has no effect on the parent if run in the child.
8619       buf[strlen(buf)-1] = 0;  // chop \n
8620       if(chdir(buf+3) < 0)
8621         printf(2, "cannot cd %s\n", buf+3);
8622       continue;
8623     }
8624     if(fork1() == 0)
8625       runcmd(parsecmd(buf));
8626     wait();
8627   }
8628   exit();
8629 }
8630 
8631 void
8632 panic(char *s)
8633 {
8634   printf(2, "%s\n", s);
8635   exit();
8636 }
8637 
8638 int
8639 fork1(void)
8640 {
8641   int pid;
8642 
8643   pid = fork();
8644   if(pid == -1)
8645     panic("fork");
8646   return pid;
8647 }
8648 
8649 
8650 // Constructors
8651 
8652 struct cmd*
8653 execcmd(void)
8654 {
8655   struct execcmd *cmd;
8656 
8657   cmd = malloc(sizeof(*cmd));
8658   memset(cmd, 0, sizeof(*cmd));
8659   cmd->type = EXEC;
8660   return (struct cmd*)cmd;
8661 }
8662 
8663 struct cmd*
8664 redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
8665 {
8666   struct redircmd *cmd;
8667 
8668   cmd = malloc(sizeof(*cmd));
8669   memset(cmd, 0, sizeof(*cmd));
8670   cmd->type = REDIR;
8671   cmd->cmd = subcmd;
8672   cmd->file = file;
8673   cmd->efile = efile;
8674   cmd->mode = mode;
8675   cmd->fd = fd;
8676   return (struct cmd*)cmd;
8677 }
8678 
8679 struct cmd*
8680 pipecmd(struct cmd *left, struct cmd *right)
8681 {
8682   struct pipecmd *cmd;
8683 
8684   cmd = malloc(sizeof(*cmd));
8685   memset(cmd, 0, sizeof(*cmd));
8686   cmd->type = PIPE;
8687   cmd->left = left;
8688   cmd->right = right;
8689   return (struct cmd*)cmd;
8690 }
8691 
8692 
8693 
8694 
8695 
8696 
8697 
8698 
8699 
8700 struct cmd*
8701 listcmd(struct cmd *left, struct cmd *right)
8702 {
8703   struct listcmd *cmd;
8704 
8705   cmd = malloc(sizeof(*cmd));
8706   memset(cmd, 0, sizeof(*cmd));
8707   cmd->type = LIST;
8708   cmd->left = left;
8709   cmd->right = right;
8710   return (struct cmd*)cmd;
8711 }
8712 
8713 struct cmd*
8714 backcmd(struct cmd *subcmd)
8715 {
8716   struct backcmd *cmd;
8717 
8718   cmd = malloc(sizeof(*cmd));
8719   memset(cmd, 0, sizeof(*cmd));
8720   cmd->type = BACK;
8721   cmd->cmd = subcmd;
8722   return (struct cmd*)cmd;
8723 }
8724 
8725 
8726 
8727 
8728 
8729 
8730 
8731 
8732 
8733 
8734 
8735 
8736 
8737 
8738 
8739 
8740 
8741 
8742 
8743 
8744 
8745 
8746 
8747 
8748 
8749 
8750 // Parsing
8751 
8752 char whitespace[] = " \t\r\n\v";
8753 char symbols[] = "<|>&;()";
8754 
8755 int
8756 gettoken(char **ps, char *es, char **q, char **eq)
8757 {
8758   char *s;
8759   int ret;
8760 
8761   s = *ps;
8762   while(s < es && strchr(whitespace, *s))
8763     s++;
8764   if(q)
8765     *q = s;
8766   ret = *s;
8767   switch(*s){
8768   case 0:
8769     break;
8770   case '|':
8771   case '(':
8772   case ')':
8773   case ';':
8774   case '&':
8775   case '<':
8776     s++;
8777     break;
8778   case '>':
8779     s++;
8780     if(*s == '>'){
8781       ret = '+';
8782       s++;
8783     }
8784     break;
8785   default:
8786     ret = 'a';
8787     while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
8788       s++;
8789     break;
8790   }
8791   if(eq)
8792     *eq = s;
8793 
8794   while(s < es && strchr(whitespace, *s))
8795     s++;
8796   *ps = s;
8797   return ret;
8798 }
8799 
8800 int
8801 peek(char **ps, char *es, char *toks)
8802 {
8803   char *s;
8804 
8805   s = *ps;
8806   while(s < es && strchr(whitespace, *s))
8807     s++;
8808   *ps = s;
8809   return *s && strchr(toks, *s);
8810 }
8811 
8812 struct cmd *parseline(char**, char*);
8813 struct cmd *parsepipe(char**, char*);
8814 struct cmd *parseexec(char**, char*);
8815 struct cmd *nulterminate(struct cmd*);
8816 
8817 struct cmd*
8818 parsecmd(char *s)
8819 {
8820   char *es;
8821   struct cmd *cmd;
8822 
8823   es = s + strlen(s);
8824   cmd = parseline(&s, es);
8825   peek(&s, es, "");
8826   if(s != es){
8827     printf(2, "leftovers: %s\n", s);
8828     panic("syntax");
8829   }
8830   nulterminate(cmd);
8831   return cmd;
8832 }
8833 
8834 struct cmd*
8835 parseline(char **ps, char *es)
8836 {
8837   struct cmd *cmd;
8838 
8839   cmd = parsepipe(ps, es);
8840   while(peek(ps, es, "&")){
8841     gettoken(ps, es, 0, 0);
8842     cmd = backcmd(cmd);
8843   }
8844   if(peek(ps, es, ";")){
8845     gettoken(ps, es, 0, 0);
8846     cmd = listcmd(cmd, parseline(ps, es));
8847   }
8848   return cmd;
8849 }
8850 struct cmd*
8851 parsepipe(char **ps, char *es)
8852 {
8853   struct cmd *cmd;
8854 
8855   cmd = parseexec(ps, es);
8856   if(peek(ps, es, "|")){
8857     gettoken(ps, es, 0, 0);
8858     cmd = pipecmd(cmd, parsepipe(ps, es));
8859   }
8860   return cmd;
8861 }
8862 
8863 struct cmd*
8864 parseredirs(struct cmd *cmd, char **ps, char *es)
8865 {
8866   int tok;
8867   char *q, *eq;
8868 
8869   while(peek(ps, es, "<>")){
8870     tok = gettoken(ps, es, 0, 0);
8871     if(gettoken(ps, es, &q, &eq) != 'a')
8872       panic("missing file for redirection");
8873     switch(tok){
8874     case '<':
8875       cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
8876       break;
8877     case '>':
8878       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
8879       break;
8880     case '+':  // >>
8881       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
8882       break;
8883     }
8884   }
8885   return cmd;
8886 }
8887 
8888 
8889 
8890 
8891 
8892 
8893 
8894 
8895 
8896 
8897 
8898 
8899 
8900 struct cmd*
8901 parseblock(char **ps, char *es)
8902 {
8903   struct cmd *cmd;
8904 
8905   if(!peek(ps, es, "("))
8906     panic("parseblock");
8907   gettoken(ps, es, 0, 0);
8908   cmd = parseline(ps, es);
8909   if(!peek(ps, es, ")"))
8910     panic("syntax - missing )");
8911   gettoken(ps, es, 0, 0);
8912   cmd = parseredirs(cmd, ps, es);
8913   return cmd;
8914 }
8915 
8916 struct cmd*
8917 parseexec(char **ps, char *es)
8918 {
8919   char *q, *eq;
8920   int tok, argc;
8921   struct execcmd *cmd;
8922   struct cmd *ret;
8923 
8924   if(peek(ps, es, "("))
8925     return parseblock(ps, es);
8926 
8927   ret = execcmd();
8928   cmd = (struct execcmd*)ret;
8929 
8930   argc = 0;
8931   ret = parseredirs(ret, ps, es);
8932   while(!peek(ps, es, "|)&;")){
8933     if((tok=gettoken(ps, es, &q, &eq)) == 0)
8934       break;
8935     if(tok != 'a')
8936       panic("syntax");
8937     cmd->argv[argc] = q;
8938     cmd->eargv[argc] = eq;
8939     argc++;
8940     if(argc >= MAXARGS)
8941       panic("too many args");
8942     ret = parseredirs(ret, ps, es);
8943   }
8944   cmd->argv[argc] = 0;
8945   cmd->eargv[argc] = 0;
8946   return ret;
8947 }
8948 
8949 
8950 // NUL-terminate all the counted strings.
8951 struct cmd*
8952 nulterminate(struct cmd *cmd)
8953 {
8954   int i;
8955   struct backcmd *bcmd;
8956   struct execcmd *ecmd;
8957   struct listcmd *lcmd;
8958   struct pipecmd *pcmd;
8959   struct redircmd *rcmd;
8960 
8961   if(cmd == 0)
8962     return 0;
8963 
8964   switch(cmd->type){
8965   case EXEC:
8966     ecmd = (struct execcmd*)cmd;
8967     for(i=0; ecmd->argv[i]; i++)
8968       *ecmd->eargv[i] = 0;
8969     break;
8970 
8971   case REDIR:
8972     rcmd = (struct redircmd*)cmd;
8973     nulterminate(rcmd->cmd);
8974     *rcmd->efile = 0;
8975     break;
8976 
8977   case PIPE:
8978     pcmd = (struct pipecmd*)cmd;
8979     nulterminate(pcmd->left);
8980     nulterminate(pcmd->right);
8981     break;
8982 
8983   case LIST:
8984     lcmd = (struct listcmd*)cmd;
8985     nulterminate(lcmd->left);
8986     nulterminate(lcmd->right);
8987     break;
8988 
8989   case BACK:
8990     bcmd = (struct backcmd*)cmd;
8991     nulterminate(bcmd->cmd);
8992     break;
8993   }
8994   return cmd;
8995 }
8996 
8997 
8998 
8999 
