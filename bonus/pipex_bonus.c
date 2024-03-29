/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afenzl <afenzl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/13 17:31:52 by afenzl            #+#    #+#             */
/*   Updated: 2022/06/28 13:31:53 by afenzl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex_bonus.h"

void	ft_here_doc(t_pipe *pipes)
{
	char	*line;

	pipes->lim = ft_strjoin(pipes->argv[2], "\n");
	dup2(pipes->fd[1], STDOUT_FILENO);
	ft_close(pipes->fd);
	line = ft_strdup("");
	while (ft_strncmp(line, pipes->lim, ft_strlen(pipes->lim)) != 0)
	{
		free(line);
		line = get_next_line(0);
		if (ft_strncmp(line, pipes->lim, ft_strlen(pipes->lim)) != 0)
			ft_putstr_fd(line, 1);
	}
	free(line);
	free(pipes->lim);
	exit(0);
}

void	first_command(t_pipe *pipes)
{
	char	*path;
	char	**cmd;
	int		fd_infile;

	dup2(pipes->fd[1], STDOUT_FILENO);
	ft_close(pipes->fd);
	fd_infile = open(pipes->argv[1], O_RDONLY);
	if (fd_infile < 0)
		ft_error(5);
	dup2(fd_infile, STDIN_FILENO);
	close(fd_infile);
	cmd = ft_split(pipes->argv[2], ' ');
	path = ft_get_path(pipes->env, cmd[0]);
	if (execve(path, cmd, NULL) < 0)
		ft_error(3);
	exit(1);
}

void	between_command(t_pipe *pipes, int cmd_num)
{
	int		id;
	char	*path;
	char	**cmd;

	if (pipe(pipes->fd) == -1)
		ft_error(-1);
	id = fork();
	if (id == -1)
		ft_error(2);
	if (id == 0)
	{
		dup2(pipes->fd_prev[0], STDIN_FILENO);
		dup2(pipes->fd[1], STDOUT_FILENO);
		ft_close(pipes->fd);
		ft_close(pipes->fd_prev);
		cmd = ft_split(pipes->argv[cmd_num], ' ');
		path = ft_get_path(pipes->env, cmd[0]);
		if (execve(path, cmd, NULL) < 0)
			ft_error(3);
		exit(1);
	}
	ft_close(pipes->fd_prev);
	assign_prev(pipes);
	ft_close(pipes->fd);
}

void	last_command(t_pipe *pipes)
{
	char	**cmd;
	char	*path;
	int		fd_outfile;

	dup2(pipes->fd_prev[0], STDIN_FILENO);
	ft_close(pipes->fd_prev);
	if (pipes->here_doc != 0)
		fd_outfile = open(pipes->argv[pipes->argc - 1], O_CREAT | O_WRONLY
				| O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	else
		fd_outfile = open(pipes->argv[pipes->argc - 1], O_CREAT | O_WRONLY
				| O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd_outfile < 0)
		ft_error(5);
	dup2(fd_outfile, STDOUT_FILENO);
	close(fd_outfile);
	cmd = ft_split(pipes->argv[pipes->argc - 2], ' ');
	path = ft_get_path(pipes->env, cmd[0]);
	if (execve(path, cmd, NULL) < 0)
		ft_error(4);
	exit(1);
}

void	ft_exec_cmd(t_pipe *pipes, int *id, int i)
{
	id[0] = fork();
	if (id[0] == -1)
		ft_error(2);
	if (id[0] == 0)
	{
		if (pipes->here_doc == 0)
			ft_here_doc(pipes);
		else
			first_command(pipes);
	}
	assign_prev(pipes);
	ft_close(pipes->fd);
	while (i < pipes->cmd_num)
	{
		between_command(pipes, i + 3);
		i++;
	}
	id[1] = fork();
	if (id[1] == -1)
		ft_error(2);
	if (id[1] == 0)
		last_command(pipes);
	ft_close(pipes->fd_prev);
}
