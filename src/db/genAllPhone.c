#include<stdio.h>
#include<stdlib.h>
int main(int argc, char* argv[])
{

	char ans;
	char output;
	int num[10];
	int i[20];
	num[0]=8;
	num[1]=0;
	num[2]=5;
	// num[3]=5;

	FILE *fp=NULL;
	ans='0';
	output='0';
	if(argc==1)
	{
		printf("output to stdout? (y | n)\n");
	}
	else if(argc==2)
	{
		printf("output to %s? (y | n) \n ",argv[1]);

		fp = fopen(argv[1], "w");
	}
	else if(argc>2)
	{
		printf("output undefined. exitting\n");
		return 0;
	}

	output=getchar();
	getchar();
	printf("You said: %c\n\n",output);


	if(output=='y' || output=='Y') 
	{

		printf("only print phone numbers beginning with (805)- 55.");
		printf("(y | n)\n");
		ans=getchar();
		getchar();

		if(ans=='y' || ans=='Y') 
		{

			printf("You said: %c\n\n",ans);
			num[3]=5;
			num[4]=5;
			for(i[2]=0;i[2]<10;i[2]++)
			{
				num[5]=i[2];
				for(i[3]=0;i[3]<10;i[3]++)
				{
					num[6]=i[3];
					for(i[4]=0;i[4]<10;i[4]++)
					{
						num[7]=i[4];
						for(i[5]=0;i[5]<10;i[5]++)
						{
							num[8]=i[5];
							for(i[6]=0;i[6]<10;i[6]++)
							{
								num[9]=i[6];
								if(fp)
								{
									fprintf(fp,"%d%d%d%d%d%d%d%d%d%d\n",num[0],num[1],num[2],num[3],num[4],num[5],num[6],num[7],num[8],num[9]); 
								}
								else
								{
									printf("%d%d%d%d%d%d%d%d%d%d\n",num[0],num[1],num[2],num[3],num[4],num[5],num[6],num[7],num[8],num[9]); 
								}
							}	 
						}
					}
				}
			}
		}
		else
		{
			printf("only print phone numbers with area code (%d%d%d).",num[0],num[1],num[2]);
			printf("? (y | n)\n");
			ans=getchar();
			getchar();
			printf("You said: %c\n\n",ans);

			if(ans=='y' || ans=='Y') 
			{
				for(i[0]=0;i[0]<10;i[0]++)
				{
					num[3]=i[0];
					for(i[1]=0;i[1]<10;i[1]++)
					{ 
						num[4]=i[1];
						for(i[2]=0;i[2]<10;i[2]++)
						{
							num[5]=i[2];
							for(i[3]=0;i[3]<10;i[3]++)
							{
								num[6]=i[3];
								for(i[4]=0;i[4]<10;i[4]++)
								{
									num[7]=i[4];
									for(i[5]=0;i[5]<10;i[5]++)
									{
										num[8]=i[5];
										for(i[6]=0;i[6]<10;i[6]++)
										{
											num[9]=i[6];
											if(fp)
											{
												fprintf(fp,"%d%d%d%d%d%d%d%d%d%d\n",num[0],num[1],num[2],num[3],num[4],num[5],num[6],num[7],num[8],num[9]); 
											}
											else
											{
												printf("%d%d%d%d%d%d%d%d%d%d\n",num[0],num[1],num[2],num[3],num[4],num[5],num[6],num[7],num[8],num[9]); 
											}
										}	 
									}
								}
							}
						}
					}
				}
			}
			else
			{

				printf("Printing ALL phone numbers\n"); 
				printf("Is that okay? (y | n)\n");
				ans=getchar();
				getchar();
				printf("You said: %c\n\n",ans);
				if(ans=='y' || ans=='Y') 
				{
					printf("Only printing phone number starting with 80******** and going to 84******** .. too bad\n");
					for(i[0] =8;i[0]<9;i[0]++)
					{
						num[0]=i[0];
						for(i[1]=0;i[1]<5;i[1]++)
						{ 
							num[1]=i[1];
							for(i[2]=0;i[2]<10;i[2]++)
							{
								num[2]=i[2];
								for(i[3]=0;i[3]<10;i[3]++)
								{
									num[3]=i[3];
									for(i[4]=0;i[4]<10;i[4]++)
									{ 
										num[4]=i[4];
										for(i[5]=0;i[5]<10;i[5]++)
										{
											num[5]=i[5];
											for(i[6]=0;i[6]<10;i[6]++)
											{
												num[6]=i[6];
												for(i[7]=0;i[7]<10;i[7]++)
												{
													num[7]=i[7];
													for(i[8]=0;i[8]<10;i[8]++)
													{
														num[8]=i[8];
														for(i[9]=0;i[9]<10;i[9]++)
														{
															num[9]=i[9];
															if(fp)
															{
																fprintf(fp,"%d%d%d%d%d%d%d%d%d%d\n",num[0],num[1],num[2],num[3],num[4],num[5],num[6],num[7],num[8],num[9]); 
															}
															else
															{
																printf("%d%d%d%d%d%d%d%d%d%d\n",num[0],num[1],num[2],num[3],num[4],num[5],num[6],num[7],num[8],num[9]); 
															}
														}	 
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
